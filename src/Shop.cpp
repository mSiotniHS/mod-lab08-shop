#include "Shop.h"
#include <thread>
#include <iostream>

using std::mutex;
using std::lock_guard;

using std::cout;

Shop::Shop(unsigned int checkoutCount,
		   milliseconds checkoutItemProcessDuration,
		   unsigned int queueMaxLength)
	: _shouldWork(true)
	, _shouldStart(false)
	, _checkoutItemProcessDuration(checkoutItemProcessDuration)
	, _checkouts()
	, _queue()
	, _queueMaxLength(queueMaxLength)
	, _data()
	, _customerIncomeTimeTable()
	, _workThread([&] { workCycle(); })
{
	for (int i = 0; i < checkoutCount; i++)
	{
		_checkouts.push_back(
				std::make_unique<Checkout>(
						Checkout(i, checkoutItemProcessDuration)));
	}

	_shouldStart = true;
}

bool Shop::tryEnqueue(const shared_ptr<Customer>& customer)
{
	if (_queue.size() == _queueMaxLength)
	{
		cout << "Rejected customer #" << customer->getId() << '\n';
		_data.rejectedCustomerCount++;
		return false;
	}
	else
	{
		cout << "Accepted customer #" << customer->getId() << '\n';
		_data.acceptedCustomerCount++;
		_customerIncomeTimeTable[customer->getId()] = std::chrono::steady_clock::now();
		_queue.push(customer);
		return true;
	}
}

void Shop::handleCustomer(const shared_ptr<Customer> &newCustomer)
{
	cout << "Got new customer! id=" << newCustomer->getId() << ", count=" << newCustomer->getItemCount() << '\n';

	lock_guard<mutex> lock(_handlerMutex);
	tryEnqueue(newCustomer);
}

void Shop::workCycle()
{
	while (_shouldWork || !_queue.empty())
	{
		if (!_shouldStart) continue;

		auto maybeCheckout = findFreeCheckout();

		if (!_queue.empty() && maybeCheckout.has_value())
		{
			lock_guard<mutex> lock(_workMutex);

			auto checkout = maybeCheckout.value();

			auto customer = _queue.front();
			_queue.pop();

			calculateAndSaveWaitTimeFor(customer);

			cout << "Sent client #" << customer->getId() << " to checkout #" << checkout->getId() << '\n';
			checkout->serve(customer);
		}
	}

	for (auto &checkout : _checkouts)
	{
		checkout->stopIfWorking();
		_data.actualWorkTimeAcrossCheckouts += checkout->getActualWorkTime();
		_data.totalOperationTimeAcrossCheckouts += checkout->getTotalOperationTime();
	}
}

void Shop::calculateAndSaveWaitTimeFor(const std::shared_ptr<Customer>& customer)
{
	auto now = std::chrono::steady_clock::now();
	auto incomeMoment = _customerIncomeTimeTable[customer->getId()];

	auto inQueueDuration = std::chrono::duration_cast<duration<float>>(now - incomeMoment);
	auto inCheckoutDuration = customer->getItemCount() * _checkoutItemProcessDuration;

	_data.totalWaitTime += inQueueDuration + inCheckoutDuration;

	_customerIncomeTimeTable.erase(customer->getId());
}

std::optional<shared_ptr<Checkout>> Shop::findFreeCheckout()
{
	for (auto &checkout : _checkouts)
	{
		if (checkout->isBusy()) continue;

		return {checkout};
	}

	return std::nullopt;
}

Shop::~Shop()
{
	stopIfWorking();
}

std::optional<CollectedData> Shop::getData() const
{
	if (isWorking())
	{
		return std::nullopt;
	}

	return {_data};
}

void Shop::stopIfWorking()
{
	if (isWorking())
	{
		_shouldWork = false;
		_workThread.join();
	}
}

bool Shop::isWorking() const
{
	return _shouldWork || !_queue.empty();
}

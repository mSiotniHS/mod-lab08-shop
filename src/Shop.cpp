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
	, _checkouts()
	, _queue()
	, _queueMaxLength(queueMaxLength)
	, _data()
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

			cout << "Sent client #" << customer->getId() << " to checkout #" << checkout->getId() << '\n';
			checkout->serve(customer);
		}
	}
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
	if (_shouldWork || !_queue.empty())
	{
		_shouldWork = false;
		_workThread.join();
	}
}

CollectedData &Shop::getData()
{
	_shouldWork = false;
	_workThread.join();

	return _data;
}

#include "Checkout.h"

Checkout::Checkout(int id, milliseconds itemProcessDuration)
	: _id(id)
	, _itemProcessDuration(itemProcessDuration)
	, _shouldWork(true)
	, _workload(std::nullopt)
	, _thread([&] { workCycle(); })
	, _mutex() {}

void Checkout::serve(const shared_ptr<Customer> &customer)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_workload = customer;
}

void Checkout::workCycle()
{
	while (_shouldWork)
	{
		if (_workload.has_value())
		{
			auto customer = _workload.value();
			std::this_thread::sleep_for(_itemProcessDuration * customer->getItemCount());

			std::lock_guard<std::mutex> lock(_mutex);
			_workload.reset();
		}
	}
}

bool Checkout::isBusy() const
{
	return _workload.has_value();
}

Checkout::Checkout(const Checkout &checkout)
	: Checkout(checkout._id, checkout._itemProcessDuration) {}

Checkout::~Checkout()
{
	_shouldWork = false;
	_thread.join();
}

int Checkout::getId() const
{
	return _id;
}

#pragma once

#include "Customer.h"
#include "Checkout.h"
#include <memory>
#include <queue>
#include <vector>
#include <mutex>
#include <optional>

using std::shared_ptr;
using std::unique_ptr;

struct CollectedData
{
public:
	unsigned int rejectedCustomerCount = 0;
	unsigned int acceptedCustomerCount = 0;
};

class Shop
{
private:
	std::mutex _handlerMutex;
	std::mutex _workMutex;
	bool _shouldWork;
	bool _shouldStart;
	std::thread _workThread;
	std::vector<shared_ptr<Checkout>> _checkouts;
	std::queue<shared_ptr<Customer>> _queue;
	unsigned int _queueMaxLength;
	CollectedData _data;

	void workCycle();
	std::optional<shared_ptr<Checkout>> findFreeCheckout();

public:
	Shop(unsigned int checkoutCount,
		 milliseconds checkoutItemProcessDuration,
		 unsigned int queueMaxLength);

	~Shop();

	/**
     * Предпринимает попытку добавить очередного клиента в очередь.
     * Если не достигнут %_queueMaxLength, клиент добавляется в очередь;
     * иначе нет.
     * @param customer Клиент для обслуживания
     * @return true, если добавить в очередь удалось; иначе false.
    */
	bool tryEnqueue(const shared_ptr<Customer>& customer);
	void handleCustomer(const shared_ptr<Customer>& newCustomer);
	CollectedData& getData();
};

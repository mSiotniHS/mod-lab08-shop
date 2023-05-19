#include "Calculator.h"

Statistics Calculator::calculateStats(
		double arrivalRate,
		double serviceRate,
		unsigned int channelCount,
		unsigned int queueMaxLength)
{
	double rho = arrivalRate / serviceRate;

	Statistics stats{};

	double term = 1;
	double noQueueProbability = term;
	for (long unsigned int i = 1; i <= channelCount + queueMaxLength; i++)
	{
		if (i <= channelCount)
		{
			term *= rho / i;
		}
		else
		{
			term *= rho / channelCount;
		}

		noQueueProbability += term;
	}
	noQueueProbability = 1 / noQueueProbability;

	stats.rejectionProbability
			= noQueueProbability
			  * pow(rho, channelCount + queueMaxLength)
			  / (factorial(channelCount) * pow(channelCount, queueMaxLength));

	stats.relativeThroughput = 1 - stats.rejectionProbability;
	stats.absoluteThroughput = arrivalRate * stats.relativeThroughput;

	return stats;
}

long unsigned int Calculator::factorial(unsigned int n)
{
	unsigned int value = 1;

	for (unsigned int i = 2; i <= n; i++)
	{
		value *= i;
	}

	return value;
}

double Calculator::pow(double x, unsigned int n)
{
	double raised = 1;

	for (unsigned int i = 1; i <= n; i++)
	{
		raised *= x;
	}

	return raised;
}

unsigned int Calculator::pow(unsigned int x, unsigned int n)
{
	unsigned int raised = 1;

	for (unsigned int i = 1; i <= n; i++)
	{
		raised *= x;
	}

	return raised;
}

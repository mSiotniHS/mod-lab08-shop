#pragma once

struct Statistics
{
public:
	double rejectionProbability;
	double relativeThroughput;
	double absoluteThroughput;
};

class Calculator
{
public:
	static Statistics calculateStats(
			double arrivalRate,
			double serviceRate,
			unsigned int channelCount,
			unsigned int queueMaxLength);

private:
	static long unsigned int factorial(unsigned int n);

	static double pow(double x, unsigned int n);
	static unsigned int pow(unsigned int x, unsigned int n);
};

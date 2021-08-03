/*
 * @Author: �޷ɽ�
 * @File: g_a.cpp
 * @Date: 2021-7-10
 * @LastEditTime: 2021-7-30
 * @LastEditors: �޷ɽ�
 * @brief: ����Ⱥ�ص��Ⱥ����㷨�����Ŵ��㷨
 */
#include <random>
#include <vector>
#include <iostream>
#include <cmath>
#include <windows.h>
#include <tchar.h>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <atlstr.h>
#include <assert.h>
#include <chrono>
#include "transit.h"
#include "pattern.h"
extern bool IS_STOP_ALL;
using namespace std;
volatile bool gaIsFinish = false;

// �������
const double CROSSOVER_PROBABILITY = 0.75;
// �������
const double MUTATION_PROBABILITY = 0.1;
// �������
const unsigned int SEED = 20;
// ��Ⱥ��ģ���ɱ�
int POPULATION_SIZE = 100;
// �����������ɱ�
int NUM_EVOLUTION = 80;

// ����
class Individual
{
private:
	// ���У�������Elevators->elevators�����е�����
	int* sequence;
	// ������Ԫ�صĸ���
	int num;
	// ��Ӧ��
	double fitness;
	// ��Ӧ�ȸ���
	double fitnessPro;
	// �ۼ���Ӧ�ȸ���
	double fitnessSumPro;
public:
	Individual(int* paramSequence, Signal* signal);
	// ��ȡ����
	int* GetSequence()
	{
		return sequence;
	}
	double GetFitness()
	{
		return fitness;
	}
	double GetFitnessPro()
	{
		return fitnessPro;
	}
	double GetFitnessSumPro()
	{
		return fitnessSumPro;
	}
	// ���ò���
	double SetFitness(const double param)
	{
		fitness = param;
		return fitness;
	}
	double SetFitnessPro(const double param)
	{
		fitnessPro = param;
		return fitnessPro;
	}
	double SetFitnessSumPro(const double param)
	{
		fitnessSumPro = param;
		return fitnessSumPro;
	}
	Individual(const Individual& user)
	{
		this->num = user.num;
		this->sequence = (int*)malloc(num * sizeof(int));
		for (int i = 0; i < num; ++i)
		{
			this->sequence[i] = user.sequence[i];
		}
		this->fitness = user.fitness;
		this->fitnessPro = user.fitnessPro;
		this->fitnessSumPro = user.fitnessSumPro;
	}
	~Individual() { free(sequence); };
};
class Const
{
public:
	const int constNum;
	Const(int num) :constNum(num) {};
};

Individual::Individual(int* paramSequence, Signal* pSignal)
{
	sequence = (int*)malloc(pSignal->signalNum * sizeof(int));
	num = pSignal->signalNum;
	for (int i = 0; i < num; ++i)
	{
		sequence[i] = paramSequence[i];
	}
	fitness = 0;
	fitnessPro = 0;
	fitnessSumPro = 0;
}

// ��ŵ�������
vector<Individual>nowPopulation;
// ���ѡ�����м����
vector<Individual>midPopulation;
// �����һ������
vector<Individual>nextPopulation;

double random()
{
	int n = rand() % 999;
	// �������0��1��С��
	return double(n) / 1000.0;
}

// ���ɳ�ʼ��Ⱥ
void Initialize(Elevators* pElevators, Signal* pSignal)
{
	int** temp = new int* [POPULATION_SIZE];
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		// ������� Elevators �и�������elevators�����е�����
		temp[i] = new int[pSignal->signalNum];
	}
	// ���������
	default_random_engine e(static_cast<unsigned>(20));
	// ��̬���� ����
	Const num(pElevators->numElevators);
	// ����[0,  pElevators->numElevators - 1] ����
	uniform_int_distribution<int>u(0, num.constNum - 1);
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		for (int n = 0; n < pSignal->signalNum; ++n)
		{
			// �����أ�һ�����ݿ�����Ҫ��Ӧ����ź�
			temp[i][n] = u(e);
		}
	}
	// �����ʼ��Ⱥ
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		int* tempTemp = new int[pSignal->signalNum];
		for (int n = 0; n < pSignal->signalNum; ++n)
		{
			tempTemp[n] = temp[i][n];
		}
		Individual individual(tempTemp, pSignal);
		nowPopulation.push_back(individual);
		delete[]tempTemp;
	}
	// ��̬�ڴ��ͷ�
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		delete[]temp[i];
	}
	delete[]temp;
}

// �м亯�����������������
double CalDistanceTime(double s, double totalTime, double tempTime, double distNodeConstant, double distNodeMaxAcce, Elevators* pElevators, unsigned int elevatorIndex)
{
	if (s < distNodeMaxAcce)
	{
		totalTime = tempTime + pow(32 * s / pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, (double)1 / 3);
	}
	else if (s >= distNodeMaxAcce && s < distNodeConstant)
	{
		totalTime = tempTime + (pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration *
			(1 + pow(1 + 4 * pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, 2) * s / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 3), (double)1 / 2))) /
			pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk;
	}
	else if (s >= distNodeConstant)
	{
		totalTime = tempTime + (pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed +
			pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk +
			(double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk * s) /
			((double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk);
	}
	return totalTime;
}

// ����������˳���ʱ��
double CaculateTakeTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam)
{
	// �������ʱ��Ϊ 0s ʱ����ֵΪ1�� ����ʱ��Ϊ90��ʱ���ۺ���ֵΪ0.001
	double totalTime = 0;
	int lastDistFloorIndex = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size() - 1;
	// ��ʱ��������ǵ�����ÿһĿ�Ĳ���ͣ����ʱ��
	double tempTime = (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfWait + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfOpenDoor
		+ (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfCloseDoor + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfBrake;
	// �����ܴﵽ��ٶȵľ���ڵ�
	double distNodeConstant = (pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed + pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk) /
		((double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk);
	// �����ܴﵽ������м��ٶȵľ���ڵ�
	double distNodeMaxAcce = pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 3) * 2 / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, 2);

	typedef struct Distance
	{
		int whatFloor;
		double time;
	}Distance;
	vector<Distance> temp;

	switch (pElevators->elevators[elevatorIndex].elevatorStatus.dir)
	{
	case UP:
	{
		double s = 0;
		// ��ʼ����
		s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist - pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor);
		// ��һ����ǰ���õ�ʱ��
		double initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);

		for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN)
			{
				s = 0;
				// ������ڲ��źŲ㵽����Ŀǰ���ڲ�ľ���
				Distance tempDistance;
				tempDistance.whatFloor = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist;
				tempDistance.time = 0;
				int j = i;

				while (j > 0)
				{
					s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist);
					tempDistance.time += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					j--;
				}
				tempDistance.time += initialPartTime;
				temp.push_back(tempDistance);
			}
		}
		break;
	}
	case DOWN:
	{
		// Ѱ�ҷָ��, ��һ���ַָ��
		int presentFloor = pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor;
		int intialFirstPoint = 0;
		for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN ||
				(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist < presentFloor && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN))
			{
				intialFirstPoint = i;
			}
		}

		double s = 0;
		// ��ʼ����
		s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[intialFirstPoint].dist - pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor);
		// ��һ����ǰ���õ�ʱ��
		double initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);

		for (int i = intialFirstPoint; i > 0; --i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN)
			{
				s = 0;
				// ������ڲ��źŲ㵽����Ŀǰ���ڲ�ľ���
				Distance tempDistance;
				tempDistance.whatFloor = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist;
				tempDistance.time = 0;
				int j = i;
				double s = (double)pBuildingParam->storeyHeight * (pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[intialFirstPoint].dist);
				while (j <= intialFirstPoint)
				{
					s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist);
					tempDistance.time += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					j++;
				}
				tempDistance.time += initialPartTime;
				temp.push_back(tempDistance);
			}
		}
		break;
	}
	default:
		break;
	}

	// ����
	for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.person.size(); ++i)
	{
		for (int j = 0; j < temp.size(); ++j)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.person[i].distFloor == temp[j].whatFloor)
			{
				totalTime += (double)pElevators->elevators[elevatorIndex].elevatorStatus.person[i].time + temp[j].time;
			}
		}
	}
	double x = 3 * log(10) / pow(90, 2);
	double ret = exp(-x * totalTime);
	return ret;
}

// ����ȴ�ʱ�����ۺ���ֵ
double CalculateWaitTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam)
{
	// ����ȴ�ʱ��Ϊ 0s ʱ����ֵΪ1�� �ȴ�ʱ��Ϊ60��ʱ���ۺ���ֵΪ0.001
	// ȫ���ⲿ�źŵĵȴ�ʱ��֮��
	double totalTime = 0;
	int lastDistFloorIndex = pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size() - 1;
	double tempTime = (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfWait + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfOpenDoor
		+ (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfCloseDoor + (double)pElevators->elevators[elevatorIndex].elevatorParam.timeOfBrake;
	// �����ܴﵽ��ٶȵľ���ڵ�
	double distNodeConstant = (pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed + pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedSpeed, 2) * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk) /
		((double)pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration * pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk);
	// �����ܴﵽ������м��ٶȵľ���ڵ�
	double distNodeMaxAcce = pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedAcceleration, 3) * 2 / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedJerk, 2);
	switch (pElevators->elevators[elevatorIndex].elevatorStatus.dir)
	{
	case UP:
	{
		double s = 0;
		// ��һ����ǰ���õ�ʱ��
		double initialPartTime = 0;
		s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist - pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor);
		// ���۵�һ������û�ж�����һ���ֵ�ʱ��
		initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
		if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_DOWN)
		{
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfDOWN;
		}
		else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_UP)
		{
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfUP - tempTime;
		}

		for (int i = 1; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN || pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
			{
				s = 0;
				int j = i;
				while (j > 0)
				{
					s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist);
					// s = 0 ʱ���ؼ���tempTime
					if (s != 0)
					{
						totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					}
					j--;
				}

				totalTime += initialPartTime;
				if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfDOWN;
				}
				else
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfUP;
				}
			}
		}
		break;
	}
	case DOWN:
	{
		int presentFloor = pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor;
		bool isHaveFirstPart = false;
		// Ѱ�ҷָ��, ��һ���ַָ��
		int intialFirstPoint = 0;
		for (int i = 0; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == INTERN ||
				(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist < presentFloor && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN))
			{
				isHaveFirstPart = true;
				intialFirstPoint = i;
			}
		}

		double s = 0;
		// ��һ�������õ�ʱ��
		double firstPartTime = 0;
		// ��һ����ǰ���õ�ʱ��
		double initialPartTime = 0;
		s = ABS(pElevators->elevators[elevatorIndex].elevatorStatus.presentFloor - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist) * pBuildingParam->storeyHeight;
		// ������û�е�һ���ֶ������ʱ��
		initialPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
		if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_DOWN && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist < presentFloor)
		{
			// �������Ĭ�ϴ��ڵ�һ����
			totalTime -= tempTime;
		}
		else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_DOWN && pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist >= presentFloor)
		{
			// �����ڵ�һ����
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfDOWN - tempTime;
		}
		else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].type == EXTERN_UP)
		{
			// �����ڵ�һ����
			totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex) + pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].waitTimeOfUP - tempTime;
		}

		if (isHaveFirstPart)
		{
			// �����һ�����ⲿ�źų˿͵ȴ�ʱ��
			for (int i = intialFirstPoint; i >= 0; --i)
			{
				if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN || pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
				{
					s = 0;
					int j = i;
					while (j < intialFirstPoint)
					{
						s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j + 1].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist);
						// s = 0 ʱ���ؼ���tempTime
						if (s != 0)
						{
							totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
						}
						++j;
					}

					totalTime += initialPartTime;
					if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
					{
						totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfDOWN;
					}
					else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
					{
						totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfUP;
					}
				}
			}
			s = ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[intialFirstPoint].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[0].dist) * pBuildingParam->storeyHeight;
			firstPartTime = CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
		}

		// ����ڶ����֡����������ⲿ�źų˿͵ȴ�ʱ��
		for (int i = intialFirstPoint + 1; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
		{
			if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN || pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
			{
				s = 0;
				int j = i;
				while (j > intialFirstPoint + 1)
				{
					s = pBuildingParam->storeyHeight * ABS(pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j].dist - pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[j - 1].dist);
					// s = 0 ʱ���ؼ���tempTime
					if (s != 0)
					{
						totalTime += CalDistanceTime(s, totalTime, tempTime, distNodeConstant, distNodeMaxAcce, pElevators, elevatorIndex);
					}
					j--;
				}

				totalTime += firstPartTime + initialPartTime;
				if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_DOWN)
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfDOWN;
				}
				else if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].type == EXTERN_UP)
				{
					totalTime += pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].waitTimeOfUP;
				}
			}
		}

		break;
	}
	default:
		break;
	}
	double x = 3 * log(10) / pow(60, 2);
	double ret = exp(-x * totalTime);
	return ret;
}

// ����ӵ�����ۺ���ֵ
double CaculateComfort(Elevators* pElevators, unsigned int elevatorIndex)
{
	// ��������ڵ�����Ϊ 0 ʱ���ʶ�Ϊ1������Ϊ�����ʱ���ʶ�Ϊ0.001������ e ָ�����仯
	double x = 3 * log(10) / pow(pElevators->elevators[elevatorIndex].elevatorParam.ratedCapacity, 2);
	double ret = exp(-x * pElevators->elevators[elevatorIndex].elevatorStatus.person.size());
	return ret;
}

// �����ܺ����ۺ���ֵ
double CaculateEnergyConsume(Elevators* pElevators, unsigned int elevatorIndex)
{
	int num = 0;
	// ����ͣ��������ж�, ����ͣ�����Ϊ5��ʱ���ۺ���ֵΪ0.01
	for (int i = 1; i < pElevators->elevators[elevatorIndex].elevatorStatus.distFloors.size(); ++i)
	{
		// �ܿ��ظ�
		if (pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i].dist != pElevators->elevators[elevatorIndex].elevatorStatus.distFloors[i - 1].dist)
		{
			num++;
		}
	}
	double x = 2 * log(10) / pow(5, 2);
	return exp(-x * num);
}
mutex mtx;
Elevators* InsertExternSignal(Elevators* pTempElevator, int* sequence, int sequenceIndex, Signal* pSignal, bool isIntern)
{
	mtx.lock();
	int index = sequence[sequenceIndex];
	// ���ⲿ�źų�ʼ��
	DistFloor tempDist;
	// �ⲿ�źŲ�
	tempDist.dist = pSignal->externSignal[sequenceIndex].floors;
	if (!isIntern)
	{
		if (pSignal->externSignal[sequenceIndex].dir == UP)
		{
			tempDist.type = EXTERN_UP;
		}
		else if (pSignal->externSignal[sequenceIndex].dir == DOWN)
		{
			tempDist.type = EXTERN_DOWN;
		}
	}
	else
	{
		tempDist.type = INTERN;
	}
	// ���ⲿ�źŵȴ�ʱ���ʼ��
	tempDist.waitTimeOfDOWN = 0;
	tempDist.waitTimeOfUP = 0;

	// ���ⲿ�źŲ���
	switch (pTempElevator->elevators[index].elevatorStatus.dir)
	{
		// ������µ���Ŀ�Ĳ㼯�ض����ڵ�һ����
	case UP:
	{
		switch (tempDist.type)
		{
		case EXTERN_UP:
		{
			// ���򣺵�����Ӧ�źż���Ϊ��������
			// �ⲿ�źŲ��ڵ��ݵ�ǰ������֮�ϣ������һ����
			if (tempDist.dist > pTempElevator->elevators[index].elevatorStatus.presentFloor)
			{
				// ��һ����
				// ��������ʼ��
				vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
				int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
				int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
				int ii = 0;
				while (tempDist.dist > pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor &&
					(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP))
				{
					ii++;
					if (ii == numOfFloor)
					{
						break;
					}
				}
				distIter += ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

				break;
			}
			// �ⲿ�źŲ��ڵ��ݵ�ǰ���ڲ�֮�£����뵽��������
			else
			{
				/*
				* ���򣺵�����Ӧ�źż���Ϊ�������� ��tempElevator.elevators[index].elevatorStatus.dir == UP �� tempDist.type == EXTERN_UP�����ⲿ�źŽ�������
				* �������֣� ��tempElevator.elevators[index].elevatorStatus.dir == UP �� tempDist.type == EXTERN_DOWN, ���ⲿ�źŽ�������ڶ�����
				*/
				// ��������
				// ��������ʼ��
				vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
				int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
				int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();

				int ii = 0;
				while ((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor) ||
					pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN ||
					(tempDist.dist > pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP))
				{
					ii++;
					if (ii == numOfFloor)
					{
						break;
					}
				}
				distIter += ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);
			}
			break;
		}
		case EXTERN_DOWN:
		{
			// ���뵽�ڶ�����
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int ii = 0;
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor) ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > tempDist.dist))
			{
				ii++;
				// �����ڵڶ��������������
				if (ii == numOfFloor)
				{
					break;
				}
			}
			distIter += ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		case INTERN:
		{
			// ���뵽��һ����
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int ii = 0;
			int dist = tempDist.dist;
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			while ((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP) &&
				pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist <= dist && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist >= presentFloor)
			{
				ii++;
			}
			distIter = distIter + ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		default:
			break;
		}
		break;
	}
	case DOWN:
	{
		int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
		bool isHaveFirstPart = false;
		// Ѱ�ҷָ��, ��һ���ַָ��
		int intialFirstPoint = 0;
		for (int i = 0; i < pTempElevator->elevators[index].elevatorStatus.distFloors.size(); ++i)
		{
			if (pTempElevator->elevators[index].elevatorStatus.distFloors[i].type == INTERN ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[i].dist < presentFloor && pTempElevator->elevators[index].elevatorStatus.distFloors[i].type == EXTERN_DOWN))
			{
				isHaveFirstPart = true;
				intialFirstPoint = i;
			}
		}

		switch (tempDist.type)
		{
		case EXTERN_DOWN:
		{
			// ��������ʼ��
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			// ���뵽��һ����
			if (tempDist.dist < presentFloor)
			{
				int ii = 0;
				while ((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN)
					&& pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < tempDist.dist)
				{
					ii++;
					if (ii == intialFirstPoint + 1)
					{
						break;
					}
				}
				distIter += ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

				break;
			}
			// ���뵽��������
			else
			{
				vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
				int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
				int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();

				int ii = intialFirstPoint;
				while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP ||
					((pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN) &&
						pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < presentFloor) ||
					(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > presentFloor && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist > tempDist.dist))
				{
					ii++;
					if (ii == numOfFloor)
					{
						break;
					}
				}
				distIter = distIter + ii;
				pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);
			}
			break;
		}
		case EXTERN_UP:
		{
			// �ڶ�����
			// ��������ʼ��
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			int ii = intialFirstPoint;
			while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < presentFloor) ||
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_UP && pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist < tempDist.dist))
			{
				ii++;
				if (ii == numOfFloor)
				{
					break;
				}
			}
			distIter = distIter + ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		case INTERN:
		{
			// ���뵽��һ����
			vector<DistFloor>::iterator distIter = pTempElevator->elevators[index].elevatorStatus.distFloors.begin();
			int ii = 0;
			int dist = tempDist.dist;
			int numOfFloor = pTempElevator->elevators[index].elevatorStatus.distFloors.size();
			int presentFloor = pTempElevator->elevators[index].elevatorStatus.presentFloor;
			while (pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist <= presentFloor &&
				(pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == INTERN || pTempElevator->elevators[index].elevatorStatus.distFloors[ii].type == EXTERN_DOWN) &&
				pTempElevator->elevators[index].elevatorStatus.distFloors[ii].dist <= dist)
			{
				ii++;
			}
			distIter = distIter + ii;
			pTempElevator->elevators[index].elevatorStatus.distFloors.insert(distIter, 1, tempDist);

			break;
		}
		default:
			break;
		}
		break;
	}
	case STATIC:
	{
		pTempElevator->elevators[index].elevatorStatus.distFloors.push_back(tempDist);
		// �������з������
		if (pTempElevator->elevators[index].elevatorStatus.presentFloor < tempDist.dist)
		{
			pTempElevator->elevators[index].elevatorStatus.dir = UP;
		}
		else if (pTempElevator->elevators[index].elevatorStatus.presentFloor > tempDist.dist)
		{
			pTempElevator->elevators[index].elevatorStatus.dir = DOWN;
		}
		break;
	}
	default:
		break;
	}
	mtx.unlock();
	return pTempElevator;
}

/*
* note:::::: �����ڶ�ȡ�źż�ʱע�⴦��Ӻδ���ʼ���źż���������������������
* �����ⲿ�ź�Ŀ�Ĳ�һ������(EXTERN_UP)��С��(EXTERN_DOWN)��ǰ�źŲ�
*
*/
// ������Ӧ��
void CalculateFitness(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal)
{
	float w1, w2, w3, w4;
	double fit = 0;
	switch (pPassengerFLow->patterns)
	{
	case FREE:
	{
		w1 = 0.4;
		w2 = 0.1;
		w3 = 0.2;
		w4 = 0.3;
		break;
	}
	case BUSY_UPRUSH:
	{
		w1 = 0.7;
		w2 = 0.2;
		w3 = 0.0;
		w4 = 0.1;
		break;
	}
	case BUSY_DOWNRUSH:
	{
		w1 = 0.7;
		w2 = 0.2;
		w3 = 0.0;
		w4 = 0.1;
		break;
	}
	case BUSY_LAYER_BALANCE:
	case BUSY_LAYER_DUPLEX:
	{
		w1 = 0.3;
		w2 = 0.0;
		w3 = 0.3;
		w4 = 0.4;
		break;
	}
	default:
		break;
	}

	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		// ����һ��elevators
		Elevators tempElevator;
		tempElevator.elevators = new ElevatorRealTimeParam[pElevators->numElevators];
		tempElevator.numElevators = pElevators->numElevators;
		for (int n = 0; n < pElevators->numElevators; ++n)
		{
			tempElevator.elevators[n].elevatorParam = pElevators->elevators[n].elevatorParam;
			tempElevator.elevators[n].elevatorStatus = pElevators->elevators[n].elevatorStatus;
		} // �������
		Elevators* pTempElevator = &tempElevator;

		// ��Ŀ������distFloors��������Ӧ��
		for (int j = 0; j < pSignal->signalNum; ++j)
		{
			int index = nowPopulation.at(i).GetSequence()[j];
			/*if (!IS_VALUE_IN_SECTION(index, 0, pSignal->externSignal.size() - 1))
			{
				index = nowPopulation.at(i).GetSequence()[j] = rand() % pSignal->externSignal.size();
			}*/
			assert(IS_VALUE_IN_SECTION(index, 0, pSignal->externSignal.size() - 1));
			pTempElevator = InsertExternSignal(pTempElevator, nowPopulation.at(i).GetSequence(), j, pSignal, false);
			fit = fit + CalculateWaitTime(pTempElevator, index, pBuildingParam) * w1 +
				CaculateComfort(pTempElevator, index) * w2 + CaculateEnergyConsume(pTempElevator, index) * w3 + CaculateTakeTime(pTempElevator, index, pBuildingParam) * w4;
		}
		for (int k = 0; k < pTempElevator->numElevators; ++k)
		{
			vector<DistFloor>().swap(pTempElevator->elevators[k].elevatorStatus.distFloors);
		}
		delete[] tempElevator.elevators;
		//printf("%lf\n", fit); // ���ڲ鿴��Ӧ�ȱ仯���
		nowPopulation.at(i).SetFitness(fit);
		fit = 0;
	}
}

// ������Ӧֵ����
void CaculatePFitness()
{
	double sumFitness = 0; // ��Ӧ���ۼ�ֵ
	double temp = 0;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		sumFitness += nowPopulation.at(i).GetFitness();
	}
	for (int n = 0; n < POPULATION_SIZE; ++n)
	{
		temp = nowPopulation.at(n).GetFitness() / sumFitness;
		nowPopulation.at(n).SetFitnessPro(temp);
	}
}

// ������Ӧֵ�ۼ�ֵ
void CaculateSumFitness()
{
	double summation = 0; // �ۼ�ֵ
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		summation += nowPopulation.at(i).GetFitnessPro();
		nowPopulation.at(i).SetFitnessSumPro(summation);
	}
}

// ѡ��
void Select()
{
	double maxFitness = nowPopulation.at(0).GetFitness();
	int maxId = 0;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		if (maxFitness < nowPopulation.at(i).GetFitness())
		{
			maxFitness = nowPopulation.at(i).GetFitness();
			maxId = i;
		}
	}
	for (int i = 0; i < POPULATION_SIZE / 4; ++i)
	{
		midPopulation.push_back(nowPopulation.at(maxId));
	}
	int NEWPOPULATIONSIZE = POPULATION_SIZE - POPULATION_SIZE / 4;
	// �������������
	double* arr = new double[NEWPOPULATIONSIZE];
	//���棬�����������
	default_random_engine e(time(0));
	uniform_real_distribution<double> u(0.0, 1.0);
	for (int i = 0; i < NEWPOPULATIONSIZE; ++i)
	{
		arr[i] = u(e);
	}
	// ����ѡ��
	for (int i = 0; i < NEWPOPULATIONSIZE; ++i)
	{
		if (arr[i] < nowPopulation.at(0).GetFitnessSumPro())
		{
			midPopulation.push_back(nowPopulation.at(0));
		}
		else
		{
			for (int n = 1; n < POPULATION_SIZE; ++n)
			{
				if (arr[i] >= nowPopulation.at(n - 1).GetFitnessSumPro() && arr[i] <= nowPopulation.at(n).GetFitnessSumPro())
				{
					midPopulation.push_back(nowPopulation.at(n));
					break;
				}
			}
		}
	}
	nowPopulation.clear();
	delete[]arr;
}

// ���滥��,����һ�㽻��
void Crossover(Signal* pSignal)
{
	int num = 0; // ��¼����
	int crossPos; // ��¼����λ��
	// ��ʱ���游�׵ķ�������
	int* arr1 = new int[pSignal->signalNum];
	// ��ʱ����ĸ�׵ķ�������
	int* arr2 = new int[pSignal->signalNum];
	// ���ڴ��潻������ĸ���
	int* newArr1 = new int[pSignal->signalNum];
	int* newArr2 = new int[pSignal->signalNum];
	Const constNum(pSignal->signalNum - 1);
	default_random_engine e(time(0));
	std::uniform_int_distribution<int> r(1, constNum.constNum);
	// POPULATION_SIZE����Ϊż��
	while (num < POPULATION_SIZE - 1)
	{
		double randNum = random();
		if (randNum <= CROSSOVER_PROBABILITY)
		{
			for (int i = 0; i < pSignal->signalNum; ++i)
			{
				// �õ����׵ķ�������
				arr1[i] = midPopulation.at(num).GetSequence()[i];
				// �õ�ĸ�׵ķ�������
				arr2[i] = midPopulation.at(num + 1).GetSequence()[i];
			}
			// ���� [1, pSigal->signalNum-1] �����
			crossPos = r(e);
			// ��ȡ��������Ƭ��
			for (int n = 0; n < crossPos; ++n)
			{
				newArr1[n] = arr1[n];
				newArr2[n] = arr2[n];
			}
			for (int i = crossPos; i < pSignal->signalNum; ++i)
			{
				newArr1[i] = arr2[i];
				newArr2[i] = arr1[i];
			}
			Individual newChild1(newArr1, pSignal);
			Individual newChild2(newArr2, pSignal);
			nextPopulation.push_back(newChild1);
			nextPopulation.push_back(newChild2);
		}
		else
		{
			nextPopulation.push_back(midPopulation.at(num));
			nextPopulation.push_back(midPopulation.at(num + 1));
		}
		num += 2;
	}
	midPopulation.clear();
	delete[]arr1;
	delete[]arr2;
	delete[]newArr1;
	delete[]newArr2;
}

// ����
void Mutation(Signal* pSignal)
{
	// �������������
	int mutationPoint[2];
	int temp;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		int* tempSequence = new int[pSignal->signalNum];
		double randNum = random();
		for (int n = 0; n < pSignal->signalNum; ++n)
		{
			tempSequence[n] = nextPopulation.at(i).GetSequence()[n];
		}
		if (randNum <= MUTATION_PROBABILITY)
		{
			for (int m = 0; m < 2; ++m)
			{
				mutationPoint[m] = rand() % pSignal->signalNum;
			}
			temp = tempSequence[mutationPoint[0]];
			tempSequence[mutationPoint[0]] = tempSequence[mutationPoint[1]];
			tempSequence[mutationPoint[1]] = temp;

			Individual mutationChild(tempSequence, pSignal);
			nowPopulation.push_back(mutationChild);
		}
		else
		{
			nowPopulation.push_back(nextPopulation.at(i));
		}
		delete[]tempSequence;
	}
	nextPopulation.clear();
}

// ��ʼΪ�������У��ڶ�����Ϊextern_down��������ֹͣʱ����Ҫ�Ե����źż����д���
ElevatorRealTimeParam* ELevatorChange(ElevatorRealTimeParam* pElevators)
{
	int j = 0;
	vector<DistFloor>temp;
	while (j < pElevators->elevatorStatus.distFloors.size() && pElevators->elevatorStatus.distFloors[j].type == EXTERN_DOWN)
	{
		temp.push_back(pElevators->elevatorStatus.distFloors[j]);
		j++;
	}
	vector<DistFloor>::iterator tempTemp;
	tempTemp = temp.begin();
	for (int n = j - 1; n >= 0; --n)
	{
		pElevators->elevatorStatus.distFloors[n] = *tempTemp;
		tempTemp += 1;
	}
	return pElevators;
}

// �Ŵ��㷨�ܺ���
Dispatch GeneticAlgorithm(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal)
{
#ifdef DEBUG
	// ����ʱ��
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
#endif // DEBUG
	bool isOnlyOne = false;
	// ֻ��һ���ź�ʱ����Ҫ����һ����ͬ���ź�
	if (pSignal->signalNum == 1)
	{
		pSignal->signalNum++;
		isOnlyOne = true;
		ExternSignal tempEx;
		tempEx.id = pSignal->externSignal[0].id + 1;
		tempEx.floors = pSignal->externSignal[0].floors;
		tempEx.dir = pSignal->externSignal[0].dir;
		pSignal->externSignal.push_back(tempEx);
	}
	NUM_EVOLUTION = 150;
	// ��ʱ�����Ӧֵ
	double maxFit = 0;
	volatile unsigned int index = 0;
	// ��ʼ�������ɳ�ʼ��Ⱥ
	Initialize(pElevators, pSignal);
	for (int i = 0; i < NUM_EVOLUTION; ++i)
	{
		if (IS_STOP_ALL)
		{
			_endthreadex(0);
		}
		assert(nowPopulation.size() == 100);
		CalculateFitness(pElevators, pPassengerFLow, pBuildingParam, pSignal);
		CaculatePFitness();
		CaculateSumFitness();
		Select();
		Crossover(pSignal);
		Mutation(pSignal);
#ifdef DEBUG
		for (int m = 0; m < POPULATION_SIZE; ++m)
		{
			if (maxFit < nowPopulation.at(m).GetFitness())
			{
				maxFit = nowPopulation.at(m).GetFitness();
				index = m;
			}
		}

		CString str;
		str.Format(_T("%lf\n"), maxFit);
		OutputDebugString(str);
		//printf("%lf\n", maxFit);
#endif // DEBUG
	}

	for (int m = 0; m < POPULATION_SIZE; ++m)
	{
		//printf("%lf\n", nowPopulation.at(m).GetFitness());
		if (maxFit < nowPopulation.at(m).GetFitness())
		{
			maxFit = nowPopulation.at(m).GetFitness();
			index = m;
		}
	}
	SingleDispatch* temp = new SingleDispatch[pSignal->signalNum];
	for (int j = 0; j < pSignal->signalNum; ++j)
	{
		temp[j].elevatorId = pElevators->elevators[nowPopulation.at(index).GetSequence()[j]].elevatorParam.id;
		temp[j].signalId = pSignal->externSignal[j].id;
		temp[j].dir = pSignal->externSignal[j].dir;
		temp[j].floors = pSignal->externSignal[j].floors;
	}
	if (!isOnlyOne)
	{
		for (int i = 0; i < pSignal->signalNum; ++i)
		{
			pElevators = InsertExternSignal(pElevators, nowPopulation.at(index).GetSequence(), i, pSignal, false);
		}
	}
	else
	{
		pSignal->signalNum--;
		pSignal->externSignal.pop_back();
		pElevators = InsertExternSignal(pElevators, nowPopulation.at(index).GetSequence(), 0, pSignal, false);
	}

	vector<Individual>().swap(nowPopulation);
	vector<Individual>().swap(midPopulation);
	vector<Individual>().swap(nextPopulation);
	// ����ȫ����������
	nowPopulation.~vector();
	midPopulation.~vector();
	nextPopulation.~vector();
	Dispatch ret;
	ret.singleDispatch = temp;
	ret.dispatchNum = pSignal->signalNum;
	gaIsFinish = true;
#ifdef DEBUG
	// ����ʱ��
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
#endif // DEBUG
	return ret;
}
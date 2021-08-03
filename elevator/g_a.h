#pragma once
#ifndef _G_A_H_
#define _G_A_H_
#include "transit.h"
#include "pattern.h"
double random();
// ���ɳ�ʼ��Ⱥ
void Initialize(Elevators* pElevators, Signal* pSignal);
// �м亯�����������������
double CalDistanceTime(double s, double totalTime, double tempTime, double distNodeConstant, double distNodeMaxAcce, Elevators* pElevators, unsigned int elevatorIndex);
// ����������˳���ʱ��
double CaculateTakeTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam);
// ����ȴ�ʱ�����ۺ���ֵ
double CalculateWaitTime(Elevators* pElevators, unsigned int elevatorIndex, BuildingParam* pBuildingParam);
// ����ӵ�����ۺ���ֵ
double CaculateComfort(Elevators* pElevators, unsigned int elevatorIndex);
// �����ܺ����ۺ���ֵ
double CaculateEnergyConsume(Elevators* pElevators, unsigned int elevatorIndex);
// ������Ӧ��
void CalculateFitness(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal);
// ������Ӧֵ����
void CaculatePFitness();
// ������Ӧֵ�ۼ�ֵ
void CaculateSumFitness();
// ѡ��
void Select();
// ���滥��,����һ�㽻��
void Crossover(Signal* pSignal);
// ����
void Mutation(Signal* pSignal);
Elevators* InsertExternSignal(Elevators* pTempElevator, int* sequence, int sequenceIndex, Signal* pSignal, bool isIntern);
// ��ʼΪ�������У��ڶ�����Ϊextern_down��������ֹͣʱ����Ҫ�Ե����źż����д���
ElevatorRealTimeParam* ELevatorChange(ElevatorRealTimeParam* pElevators);
// �Ŵ��㷨�ܺ���
Dispatch GeneticAlgorithm(Elevators* pElevators, PassengerFLow* pPassengerFLow, BuildingParam* pBuildingParam, Signal* pSignal);
#endif
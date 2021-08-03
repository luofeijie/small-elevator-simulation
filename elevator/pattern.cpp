/*
 * @Author: �޷ɽ�
 * @File: pattern.cpp
 * @Date: 2021-7-7
 * @LastEditTime: 2021-7-15
 * @LastEditors: �޷ɽ�
 */
 // �Բ�ͬ�Ľ�ͨģʽѡ����ʵ�Ⱥ���㷨
 // ��ͨģʽ�ķ����� Ԥ��Ŀ���������ǰʱ�̵Ŀ�������ʵ�����е���̨���й�
 // ��ͨģʽ��ʶ�������Ҫ������¥�ڸ�¥������ٷֱ������л���
 // �������ʱ���(�����о��������), ����ʱ��εĿ����仯�� r ��ĳ���̶�������仯
 /*
 * ��ͨģʽ: ���н�ͨģʽ����æ��ͨģʽ->(�ϸ߷彻ͨģʽ����佻ͨģʽ->(��·����·��ͨģʽ��ƽ���佻ͨģʽ)���¸߷彻ͨģʽ)
 */
 /*
 * ���ݿ�������: �ܿ�����������������������������������
 */
#include "gm.h"
#include <stdio.h>
#include <cstdlib>
#include"transit.h"
#include "pattern.h"
 ////PassengerFlowVolume passengerFlowVolume;
 //// ����Ϊpassenger_flow_volume_data�ļ���һ���������ڵ�ʱ��
 //void PatternInit(PassengerFlowVolume* pPassengerFlowVolume)
 //{
 //	pPassengerFlowVolume->fileCursorPoint = 0;
 //	FILE* file;
 //	errno_t err = fopen_s(&file, "passenger_flow_volume_data", "rb");
 //	if (err != 0)
 //	{
 //		perror("Open failed!");
 //		exit(1);
 //	}
 //	fseek(file, pPassengerFlowVolume->fileCursorPoint, SEEK_SET);
 //	fread(pPassengerFlowVolume->passengerFlow, sizeof(int), MAX_VOLUME_ARR_SIZE, file);
 //	pPassengerFlowVolume->fileCursorPoint = ftell(file);
 //	fclose(file);
 //	// arr ��ʼ��
 //	for (int i = 0; i < MAX_VOLUME_ARR_SIZE; ++i)
 //	{
 //	}
 //	// ��ʼ��arr��Ϻ�, ����ʱ���߳�, �����Ӹ���һ��pPassengerFlowVolume
 //	void* timeHandle = StartupTimeThread(pPassengerFlowVolume); // handle ͳһ����transit����
 //	// �仯�ʳ�ʼ����
 //	/*pGradient->maxGradient = (arr[DATA_INTERVAL] - arr[0]) / DATA_INTERVAL;
 //	pGradient->minGradient = pGradient->maxGradient;
 //	float slope;
 //	for (int i = DATA_INTERVAL; i < MAX_VOLUME_ARR_SIZE; ++i)
 //	{
 //		slope = (arr[i] - arr[i - DATA_INTERVAL]) / DATA_INTERVAL;
 //		if (slope < pGradient->minGradient)
 //		{
 //			pGradient->minGradient = slope;
 //		}
 //		else
 //		{
 //			pGradient->maxGradient = slope;
 //		}
 //	}*/
 //}
 //// ����Ԥ������, ����������Ϊ����β������Ԥ��ֵ������
 //float* PredictCorrection(float* arr, int size, int predictNum)
 //{
 //	// k = (arr[i] - arr[i-1]) / DATA_INTERVAL;
 //	return NULL;
 //}
PassengerFlowPredict* FlowPredict(PassengerFLow* pPassengerFlow, PassengerFlowPredict* pPassengerFlowPredict)
{
	for (int i = 0; i < FLOW_VOLUME_NUM; ++i)
	{
		PassengerFlowVolume* tempVol = (PassengerFlowVolume*)P_GET_PASSENGER_FLOW_ELEM(pPassengerFlow, i);
		FlowPredictArrInfo* tempInfo = (FlowPredictArrInfo*)P_GET_PASSENGER_FLOW_PREDICT_ELEM(pPassengerFlowPredict, i);
		for (int n = REAL_FLOW_NUM - 1, m = 0; n >= 0; --n, ++m)
		{
			int index = tempVol->nowTimeIndex - 1;
			tempInfo->arr[n] = tempVol->passengerFlow[index - m];
		}
		float* temp = GM_Predict(tempInfo->arr, REAL_FLOW_NUM, REAL_FLOW_NUM + PREDICT_NUM, REAL_FLOW_NUM / 2);
		tempInfo->predictIndex = REAL_FLOW_NUM;
		for (int k = 0; k < PREDICT_NUM; ++k)
		{
			tempInfo->arr[REAL_FLOW_NUM + k] = temp[REAL_FLOW_NUM + k];
		}
	}
	return pPassengerFlowPredict;
}
Patterns PatternJudge(PassengerFLow* pPassengerFLow, PassengerFlowPredict* pPassengerFlowPredict, ElevatorParam* pElevatorParam, BuildingParam* pBuildingParam)
{
	// ʵ�ʿ�����: ��һ�׶�ʣ��(δ�ĳ���) + ���׶�����
	unsigned int maxPassengerNum = pBuildingParam->numOfElevator * pElevatorParam->ratedCapacity;
	unsigned int totalPassengerNum = pPassengerFLow->totalPassengerNum.passengerFlow[pPassengerFLow->totalPassengerNum.nowTimeIndex];
	unsigned int enterPassengerNum = pPassengerFLow->enterPassengerNum.passengerFlow[pPassengerFLow->enterPassengerNum.nowTimeIndex];
	unsigned int outPassengerNum = pPassengerFLow->outPassengerNum.passengerFlow[pPassengerFLow->outPassengerNum.nowTimeIndex];
	unsigned int totalLayerPassengerNum = pPassengerFLow->totalLayerPassengerNum.passengerFlow[pPassengerFLow->totalLayerPassengerNum.nowTimeIndex];
	unsigned int maxLayerPassengerNum = pPassengerFLow->maxLayerPassengerNum.passengerFlow[pPassengerFLow->maxLayerPassengerNum.nowTimeIndex];
	unsigned int otherLayerPassengerNum = pPassengerFLow->otherLayerPassengerNum.passengerFlow[pPassengerFLow->otherLayerPassengerNum.nowTimeIndex];
	float enterRatio = PASSENGER_FLOW_RATIO(enterPassengerNum, totalPassengerNum); // X11
	float outRatio = PASSENGER_FLOW_RATIO(outPassengerNum, totalPassengerNum); // X22
	unsigned int xl = totalPassengerNum - enterPassengerNum - outPassengerNum;
	float layerRaio = PASSENGER_FLOW_RATIO(xl, totalPassengerNum); // X33
	float maxLayerRatio = PASSENGER_FLOW_RATIO(maxLayerPassengerNum, xl); // X44
	float otherLayerRatio = PASSENGER_FLOW_RATIO(otherLayerPassengerNum, xl); // X55
	float totalRatio = PASSENGER_FLOW_RATIO(totalPassengerNum, maxPassengerNum); // X0
	const float ZERO_CONST = 0;
	const float MIN_CONST = 0.25;
	const float MID_CONST = 0.45;
	const float MAX_CONST = 0.55;
	if (totalRatio < MIN_CONST)
	{
		// ����1
		pPassengerFLow->patterns = FREE;
	}
	else
	{
		// Ԥ��(��Ԥ��ֵ�Ǹ��ݱ��׶�������Ԥ��)
		int enterPredictIndex = pPassengerFlowPredict->enterFlowPredictArrInfo.predictIndex;
		int outPredictIndex = pPassengerFlowPredict->outFlowPredictArrInfo.predictIndex;
		int layerPredictIndex = pPassengerFlowPredict->totalLayerFlowPredictArrInfo.predictIndex;
		int maxLayerPredictIndex = pPassengerFlowPredict->maxLayerFlowPredictArrInfo.predictIndex;
		int otherLayerPredictIndex = pPassengerFlowPredict->otherLayerFlowPredictArrInfo.predictIndex;
		int totalPredictIndex = pPassengerFlowPredict->totalFlowPredictArrInfo.predictIndex;
		float enterPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->enterFlowPredictArrInfo.arr[enterPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->enterFlowPredictArrInfo.arr[enterPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X11_
		float outPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->outFlowPredictArrInfo.arr[outPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->outFlowPredictArrInfo.arr[outPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X22_
		float totalLayerPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->totalLayerFlowPredictArrInfo.arr[layerPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->totalLayerFlowPredictArrInfo.arr[layerPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X33_
		float maxLayerPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->maxLayerFlowPredictArrInfo.arr[maxLayerPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->maxLayerFlowPredictArrInfo.arr[maxLayerPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X44_
		float otherLayerPassengerNumRatio = PASSENGER_FLOW_RATIO((pPassengerFlowPredict->otherLayerFlowPredictArrInfo.arr[otherLayerPredictIndex] * PREDICT_ATTRIBUTE + pPassengerFlowPredict->otherLayerFlowPredictArrInfo.arr[otherLayerPredictIndex - 1] * REAL_ATTRIBUTE), pPassengerFlowPredict->totalFlowPredictArrInfo.arr[totalPredictIndex]); // X55_

		// �ϸ߷�ģʽ: ����2��9
		if (enterRatio >= MAX_CONST ||
			(
				IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) &&
				IS_GREATER_THAN_ALL(enterPassengerNumRatio, outPassengerNumRatio, totalLayerPassengerNumRatio)
				)
			)
		{
			pPassengerFLow->patterns = BUSY_UPRUSH;
		}
		// �¸߷�ģʽ: ����3��10
		else if (outRatio >= MAX_CONST ||
			(
				IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) &&
				IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) &&
				IS_GREATER_THAN_ALL(outPassengerNumRatio, enterPassengerNumRatio, totalLayerPassengerNumRatio)
				)
			)
		{
			pPassengerFLow->patterns = BUSY_DOWNRUSH;
		}
		// ��·��ͨģʽ: ����4��6��11��13��16��18
		else if ((layerRaio >= MAX_CONST && maxLayerRatio >= MAX_CONST) ||
			(layerRaio >= MAX_CONST && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio > otherLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && maxLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio > otherLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_GREATER_THAN_ALL(layerRaio, enterRatio, outRatio) && maxLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio > otherLayerPassengerNumRatio)
			)
		{
			pPassengerFLow->patterns = BUSY_LAYER_DUPLEX;
		}
		// ƽ���佻ͨģʽ: ����5��7��8��12��14��15��17��19
		else if ((layerRaio >= MAX_CONST && otherLayerRatio >= MAX_CONST) ||
			(layerRaio >= MAX_CONST && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio <= otherLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && (enterPassengerNumRatio == outPassengerNumRatio || enterPassengerNumRatio == totalLayerPassengerNumRatio || outPassengerNumRatio == totalLayerPassengerNumRatio)) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && otherLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(outRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(layerRaio, MID_CONST, MAX_CONST) && IS_GREATER_THAN_ALL(totalLayerPassengerNumRatio, enterPassengerNumRatio, outPassengerNumRatio) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && otherLayerPassengerNumRatio >= maxLayerPassengerNumRatio) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && (enterRatio == outRatio || enterRatio == layerRaio || outRatio == layerRaio)) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_GREATER_THAN_ALL(layerRaio, enterRatio, outRatio) && otherLayerRatio >= MAX_CONST) ||
			(IS_VALUE_IN_SECTION(enterRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(outRatio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(layerRaio, ZERO_CONST, MID_CONST) && IS_VALUE_IN_SECTION(maxLayerRatio, MID_CONST, MAX_CONST) && IS_VALUE_IN_SECTION(otherLayerRatio, MID_CONST, MAX_CONST) && maxLayerPassengerNumRatio <= otherLayerPassengerNumRatio)
			)
		{
			pPassengerFLow->patterns = BUSY_LAYER_BALANCE;
		}
	}
	return pPassengerFLow->patterns;
}

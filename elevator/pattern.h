#pragma once
#ifndef _PATTERN_H_
#define _PATTERN_H_
#include "transit.h"
#define IS_SLOPE_IN_SECTION(slope, min_slope, max_slope) ((slope) >= (min_slope) && (slope) <= (max_slope))
#define ABS(v)  (v >= 0 ? v : -(v))
#define IS_SLOPE_IN_DOUBLE_MIN_SECTION(slope, min_slope, max_slope)  ((slope) < (min_slope) && ABS(slope) <= 2 * ABS(min_slope))
#define IS_SLOPE_IN_DOUBLE_MAX_SECTION(slope, min_slope, max_slope)  ((slope) > (max_slope) && ABS(slope) >= 2 * ABS(max_slope))
#define PASSENGER_FLOW_RATIO(mole, deno)  ((mole) / (deno))
#define IS_VALUE_IN_SECTION(val, val_min, val_max)  ((val) >= (val_min) && (val) <= (val_max))
#define IS_GREATER_THAN_ALL(val, src1, src2)  ((val) > (src1) && (val) > (src2))
#define PREDICT_NUM 2     // Ԥ����������Ԥ�����ݵĸ���
#define REAL_FLOW_NUM 4   // Ԥ��������ʵ���ݵĸ���
#define FLOW_VOLUME_NUM 6 // ��ͨ��������
#define P_GET_PASSENGER_FLOW_PREDICT_ELEM(pf, num)  ((char*)pf + (num) * sizeof(FlowPredictArrInfo))
#define P_GET_PASSENGER_FLOW_ELEM(pf, num)  ((char*)pf + (num) * sizeof(PassengerFlowVolume))
#define PREDICT_ATTRIBUTE 0.7 // ������Ҫ��
#define REAL_ATTRIBUTE 0.3 // ������Ҫ��
typedef struct Gradient
{
	float minGradient; // ��С�������仯��
	float maxGradient; // ���������仯��
}Gradient;
typedef enum Patterns
{
	FREE,                  // ���н�ͨģʽ
	BUSY_UPRUSH,           // ��æ-�ϸ߷彻ͨģʽ
	BUSY_DOWNRUSH,         // ��æ-�¸߷彻ͨģʽ
	BUSY_LAYER_DUPLEX,     // ��æ-���-��·��ͨģʽ
	BUSY_LAYER_BALANCE,	   // ��æ-ƽ����
}Patterns;
/*
* PassengerFlowVolumeΪ���¿������������¿�����������һʱ��ʣ�������������������
*/
typedef struct PassengerFLow
{
	PassengerFlowVolume totalPassengerNum;	    // �ܿ���
	PassengerFlowVolume enterPassengerNum;      // ����������
	PassengerFlowVolume outPassengerNum;        // ����������
	PassengerFlowVolume totalLayerPassengerNum; // �ܲ�����
	PassengerFlowVolume maxLayerPassengerNum;   // ��������
	PassengerFlowVolume otherLayerPassengerNum; // ����������
	Patterns patterns;							// ��ͨģʽ
}PassengerFLow;
typedef struct FlowPredictArrInfo
{
	float arr[REAL_FLOW_NUM + PREDICT_NUM]; // ǰ��Ϊ��ʵ���ݣ����ΪԤ������
	int predictIndex;                       // �Ӹ�������������ݽ�ΪԤ������
}FlowPredictArrInfo;
typedef struct PassengerFlowPredict
{
	FlowPredictArrInfo totalFlowPredictArrInfo;
	FlowPredictArrInfo enterFlowPredictArrInfo;
	FlowPredictArrInfo outFlowPredictArrInfo;
	FlowPredictArrInfo totalLayerFlowPredictArrInfo;
	FlowPredictArrInfo maxLayerFlowPredictArrInfo;
	FlowPredictArrInfo otherLayerFlowPredictArrInfo;
}PassengerFlowPredict;
// ��ͨģʽʶ���ʼ��
void PatternInit(PassengerFlowVolume* pPassengerFlowVolume);
// ��ͨ��Ԥ��
PassengerFlowPredict* FlowPredict(PassengerFLow* pPassengerFlow, PassengerFlowPredict* pPassengerFlowPredict);
// ��ͨģʽʶ��
Patterns PatternJudge(PassengerFLow* pPassengerFLow, PassengerFlowPredict* pPassengerFlowPredict, ElevatorParam* pElevatorParam, BuildingParam* pBuildingParam);
#endif
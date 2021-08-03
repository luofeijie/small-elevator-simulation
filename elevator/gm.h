#pragma once
#ifndef _GM_H_
#define _GM_H_

#include"matrix.h"
/* �ۼ����� */
float* GM_AGO(float* xArr, int arrSize);
/* �ۼ����� */
float* GM_IAGO(float* xArr, int arrSize);
/* ��Ȩ��ֵ����
* generateCoefficent: ����ϵ��(0.5Ϊ��ֵ����ϵ��)
* note: ���������С�ȴ��������СҪС 1
*/
float* GM_WAV_Weight(float* xArr, int arrSize, float generateCoefficent);
/*			����ֵZ(1)(k)�Ż�
* note: ���������С�ȴ��������СҪС 1
*/
float* GM_WAV_Optimize(float* xArr, int arrSize);
/* ���ݼ��鴦�� */
float* GM_DataProcess(float* xArr, int arrSize, double* c);
/* Ԥ�� */
float* GM_Predict(float* xArr, int arrSize, int retArrSize, int datumPoint);

#endif
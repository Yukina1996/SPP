/********************************************************
�ռ�����ϵת������ؼ���ĺ���ģ��
�������ܣ�������굽�ѿ��������ת��/BLHToXYZ
         �ѿ������굽��������ת��/XYZToBLH

���ߣ�������
���ڣ�2017/4/28
********************************************************/
#pragma once
#ifndef _SPACE_H
#define _SPACE_H

#include <math.h>

bool BLHToXYZ(const double BLH[], double XYZ[], const double a, const double e2);
bool XYZToBLH(const double XYZ[], double BLH[], const double a, const double e2);

#endif
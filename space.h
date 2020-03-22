/********************************************************
空间坐标系转换的相关计算的函数模块
函数功能：大地坐标到笛卡尔坐标的转换/BLHToXYZ
         笛卡尔坐标到大地坐标的转换/XYZToBLH

作者：王雅仪
日期：2017/4/28
********************************************************/
#pragma once
#ifndef _SPACE_H
#define _SPACE_H

#include <math.h>

bool BLHToXYZ(const double BLH[], double XYZ[], const double a, const double e2);
bool XYZToBLH(const double XYZ[], double BLH[], const double a, const double e2);

#endif
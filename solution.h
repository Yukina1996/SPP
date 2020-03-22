/*********************************************************************
定位模块：包括根据星历计算卫星的位置、速度和卫星钟差
		 根据星历计算电离层、对流层误差改正

作者：王雅仪
时间：2017/5/15
*********************************************************************/
#pragma once
#ifndef _SOLUTION_H
#define _SOLUTION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int calsat(const struct GPSEPHEM *ephem, const struct GPSTIME *time, int PRN, struct CALSATPOS *satpos);
int calsatEA(const double staXYZ[3], const double satXYZ[3], double *eleAngle, double *azimuth);
double klobuchar(const struct GPSTIME *time, const double staXYZ[3], double eleAngle, double azimuth, const struct IONUTC *ion);
double hopefield(const double staXYZ[3], double eleAngle); 


#endif
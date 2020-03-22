/*********************************************************************
��λģ�飺�������������������ǵ�λ�á��ٶȺ������Ӳ�
		 ���������������㡢������������

���ߣ�������
ʱ�䣺2017/5/15
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
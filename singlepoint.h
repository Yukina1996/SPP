/*********************************************************************
���㶨λģ�飺����α��۲�ֵ�����վ��λ�á��ٶȡ����ջ��Ӳ����

���ߣ�������
ʱ�䣺2017/5/18
*********************************************************************/
#pragma once
#ifndef _SINGLEPOINT_H
#define _SINGLEPOINT_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int calsta(const struct RANGE *range, const struct GPSEPHEM *ephem, const struct IONUTC *ion, struct CALSTAPOS *stapos, struct CALSATPOS satpos[]);
double calsatclk(const struct GPSTIME *time, const struct GPSEPHEM *ephem, int PRN);
int calstavelocity(const struct RANGE *range, const struct CALSATPOS satpos[], struct CALSTAPOS *stapos);


#endif
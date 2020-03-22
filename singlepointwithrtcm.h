#pragma once
#ifndef _SINGLEPOINTWITHRTCM_H
#define _SINGLEPOINTWITHRTCM_H

#include<stdio.h>
#include<stdlib.h>
#include <math.h>

int calstawithrtcm(const struct RANGE *range, const struct GPSEPHEM *ephem, const struct IONUTC *ion, struct CALSTAPOS *stapos, struct CALSATPOS satpos[], struct SATRTCM satrtcm[]);
bool IsRTCMAvailable(const struct GPSTIME* Time, short Prn, double IODE, const struct SATRTCM satrtcm[], double* prc);

#endif
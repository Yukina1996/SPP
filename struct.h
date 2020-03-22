/*********************************************************************
所有结构体的定义和所有常数的定义


作者：王雅仪
时间：2017/5/9
*********************************************************************/
#pragma once
#ifndef _STRUCT_H
#define _STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define   BYTE  unsigned char
#define NumOfChannel 14			//接收机接收卫星信号的最大通道数
#define NumOfGPS 32
#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */
#define PI 3.1415926535898
#define GM_WGS84 398600441500000			//m^3/s^2
#define a_WGS84 6378137
#define e2_WGS84 0.00669437999013
#define C 2.99792458e8 					//m/s
#define OMEGAedot 0.000072921151467		//rad/s
#define THRESHOLD 1e-10					//门限值1^-10
#define FreqL1 1.57542e9				//卫星L1信号的频率 

//定义通用时结构体
struct COMMONTIME
{
	unsigned short Year;
	unsigned short Month;
	unsigned short Day;
	unsigned short Hour;
	unsigned short Minute;
	double  Second;
};

//定义简化儒略日
struct MJDTIME
{
	int Days;
	double FracDay;

	MJDTIME()
	{
		Days = 0;
		FracDay = 0.0;
	}
};

//定义GPS时间
struct GPSTIME
{
	unsigned short Week;
	double SecOfWeek;

	GPSTIME()
	{
		Week = 0;
		SecOfWeek = 0.0;
	}
};

/*********************************************************************
与每颗卫星有关的观测数据OBSERVATION

内容包括：	卫星PRN号，伪距，载波相位，多普勒频移，
			载噪比(CND:carrier to noise density ratio),
			伪距精度，载波相位精度
*********************************************************************/
struct OBSERVATION
{
	unsigned short PRN;
	double Psr;
	float PsrStd;
	double Adr;
	float AdrStd;
	float Dopp;
	float CND;
};

/*********************************************************************
观测数据结构体RANGE

内容包括：	观测时间，卫星数
			与每颗卫星有关的观测数据
ID=43
*********************************************************************/
struct RANGE
{
	struct GPSTIME ObsGPSTime;
	long NumofObs;
	struct OBSERVATION Obs[NumOfChannel];
};

/*********************************************************************
卫星星历结构体GPSEPHEM

内容包括：	卫星PRN号，星历参考时刻
			卫星钟差参数，卫星轨道根数和摄动根数
ID=7
*********************************************************************/
struct GPSEPHEM
{
	unsigned long PRN;
	double Tow;
	unsigned long Health;
	unsigned long IODE;
	unsigned long Week;
	unsigned long zWeek;
	double Toe;
	double A;
	double deltaN;
	double M0;
	double ecc;
	double omega;
	double cuc;
	double cus;
	double crc;
	double crs;
	double cic;
	double cis;
	double I0;
	double RofI;
	double omega0;
	double RofOmega0;
	double Toc;
	double Tgd;
	double a0;
	double a1;
	double a2;
	double N;
};

/*********************************************************************
电离层改正模型结构体IONUTC

内容包括：	KLOBUCHAR模型参数

ID=8
*********************************************************************/
struct IONUTC
{
	double a0;
	double a1;
	double a2;
	double a3;
	double b0;
	double b1;
	double b2;
	double b3;
	unsigned long UtcWn;		//UTC reference week number
	unsigned long Tot;			//Reference time of UTC parameters
	double A0;					//UTC constant term of polynomial
	double A1;					//UTC 1st order term of polynomial
	unsigned long WnLsf;		//Future week number
	unsigned long Dn;			//Day number
	long deltatLs;				//Delta time due to leap seconds
	long deltatLsf;				//Future delta time due to leap seconds
	unsigned long deltatUtc;	//Time difference
};

/*********************************************************************
定位结果结构体PSRPOS

内容包括：	定位时间，位置，速度，定位精度，DOP，卫星数等?????

ID=47
*********************************************************************/
struct PSRPOS
{
	struct GPSTIME ObsGPSTime;
	double NovLat;
	double NovLon;
	double NovHgt;
	float NovUndulation;
};

/*********************************************************************
卫星位置、速度、钟差计算结果结构体CALSATPOS

内容包括：卫星位置、速度、钟差、钟速、高度角、方位角
*********************************************************************/
struct CALSATPOS
{
	unsigned short PRN;
	double satXYZ[3];
	double satXYZdot[3];
	double satClk;
	double satClkdot;
	double eleAngle;
	double azimuth;
	double ionDelay;
	double tropDelay;
};

/*********************************************************************
接收机的位置、速度、钟差计算结果结构体CALSTAPOS

内容包括：测站的位置、速度、接收机钟差、钟速、定位结果的单位权中误差
		定位的PDOP值、DOP值
*********************************************************************/
struct CALSTAPOS
{
	double staXYZ[3];
	double staBLH[3];
	double staXYZdot[3];
	double staClk;
	double staClkdot;
	double sigmaP;
	double PDOP;
	double DOP_pos;
	double sigmaV;
	double DOP_v;
};

/*********************************************************************
卫星当前差分改正数结构体SATRTCM

内容包括：卫星的PRN号、ScaleFactor、UDRE、伪距改正数、伪距变化率改正数、
		期号IOD、flag（当某段差分电文某个字奇偶校验失败时，对由这个电文解码得到
		的所有卫星的rtcm标记为1，表示不可用；若标记为0，表示可用）
*********************************************************************/
struct SATRTCM
{
	double T;				//差分改正数对应的时刻（Z计数）
	BYTE PRN;
	float RangeCorrection;
	float RangeRateCorrection;
	BYTE IOD;
	bool status;
	SATRTCM()
	{
		T=RangeCorrection=RangeRateCorrection=0;
		PRN=IOD=0;
		status=false;
	}
};

/*********************************************************************
rtcm差分改正数正文部分的数据

内容包括：这颗卫星的伪距改正数的相关信息

*********************************************************************/
struct RTCMBODY {
	BYTE scale;
	BYTE UDRE;
	BYTE SatID;
	double PRC0;
	double RRC;
	BYTE IOD;
	RTCMBODY()
	{
		scale=UDRE=SatID=0;
		PRC0=RRC=IOD=0;
	}
};

/*********************************************************************
rtcm差分改正结构体RTCMDATA

内容包括：解码出来的RTCM电文中的所有有用数据

*********************************************************************/
struct RTCMDATA
{
	BYTE type;
	unsigned short id;
	BYTE seqno;			//序号
	BYTE length;			//帧长
	float zcount;		//Z计数
	int status;			//基站健康状况
	int flag;			//表示该差分改正数是否可用，1为不可用，0为可用
	struct RTCMBODY rtcmbody[48];
	RTCMDATA()
	{
		type=id=seqno=length=zcount=flag=0;
		status=false;
	}
};

#endif
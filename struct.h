/*********************************************************************
���нṹ��Ķ�������г����Ķ���


���ߣ�������
ʱ�䣺2017/5/9
*********************************************************************/
#pragma once
#ifndef _STRUCT_H
#define _STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define   BYTE  unsigned char
#define NumOfChannel 14			//���ջ����������źŵ����ͨ����
#define NumOfGPS 32
#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */
#define PI 3.1415926535898
#define GM_WGS84 398600441500000			//m^3/s^2
#define a_WGS84 6378137
#define e2_WGS84 0.00669437999013
#define C 2.99792458e8 					//m/s
#define OMEGAedot 0.000072921151467		//rad/s
#define THRESHOLD 1e-10					//����ֵ1^-10
#define FreqL1 1.57542e9				//����L1�źŵ�Ƶ�� 

//����ͨ��ʱ�ṹ��
struct COMMONTIME
{
	unsigned short Year;
	unsigned short Month;
	unsigned short Day;
	unsigned short Hour;
	unsigned short Minute;
	double  Second;
};

//�����������
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

//����GPSʱ��
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
��ÿ�������йصĹ۲�����OBSERVATION

���ݰ�����	����PRN�ţ�α�࣬�ز���λ��������Ƶ�ƣ�
			�����(CND:carrier to noise density ratio),
			α�ྫ�ȣ��ز���λ����
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
�۲����ݽṹ��RANGE

���ݰ�����	�۲�ʱ�䣬������
			��ÿ�������йصĹ۲�����
ID=43
*********************************************************************/
struct RANGE
{
	struct GPSTIME ObsGPSTime;
	long NumofObs;
	struct OBSERVATION Obs[NumOfChannel];
};

/*********************************************************************
���������ṹ��GPSEPHEM

���ݰ�����	����PRN�ţ������ο�ʱ��
			�����Ӳ���������ǹ���������㶯����
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
��������ģ�ͽṹ��IONUTC

���ݰ�����	KLOBUCHARģ�Ͳ���

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
��λ����ṹ��PSRPOS

���ݰ�����	��λʱ�䣬λ�ã��ٶȣ���λ���ȣ�DOP����������?????

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
����λ�á��ٶȡ��Ӳ�������ṹ��CALSATPOS

���ݰ���������λ�á��ٶȡ��Ӳ���١��߶Ƚǡ���λ��
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
���ջ���λ�á��ٶȡ��Ӳ�������ṹ��CALSTAPOS

���ݰ�������վ��λ�á��ٶȡ����ջ��Ӳ���١���λ����ĵ�λȨ�����
		��λ��PDOPֵ��DOPֵ
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
���ǵ�ǰ��ָ������ṹ��SATRTCM

���ݰ��������ǵ�PRN�š�ScaleFactor��UDRE��α���������α��仯�ʸ�������
		�ں�IOD��flag����ĳ�β�ֵ���ĳ������żУ��ʧ��ʱ������������Ľ���õ�
		���������ǵ�rtcm���Ϊ1����ʾ�����ã������Ϊ0����ʾ���ã�
*********************************************************************/
struct SATRTCM
{
	double T;				//��ָ�������Ӧ��ʱ�̣�Z������
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
rtcm��ָ��������Ĳ��ֵ�����

���ݰ�����������ǵ�α��������������Ϣ

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
rtcm��ָ����ṹ��RTCMDATA

���ݰ��������������RTCM�����е�������������

*********************************************************************/
struct RTCMDATA
{
	BYTE type;
	unsigned short id;
	BYTE seqno;			//���
	BYTE length;			//֡��
	float zcount;		//Z����
	int status;			//��վ����״��
	int flag;			//��ʾ�ò�ָ������Ƿ���ã�1Ϊ�����ã�0Ϊ����
	struct RTCMBODY rtcmbody[48];
	RTCMDATA()
	{
		type=id=seqno=length=zcount=flag=0;
		status=false;
	}
};

#endif
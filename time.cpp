/********************************************************
ʱ��ת����ؼ���ĺ���ģ��
�������ܣ���ͨ��ʱת��Ϊ��������/CommonTimeToMjdTime
		 �ɼ�������ת��Ϊͨ��ʱ/MjdTimeToCommonTime
		 �ɼ�������ת��ΪGPSʱ/MjdTimeToGpsTime
		 ��GPSʱת��Ϊ��������/GpsTimeToMjdTime

���ߣ�������
���ڣ�2017/4/28
********************************************************/
#include "stdafx.h"
#include "time.h"
#include "struct.h"

/********************************************************
CommonTimeToMjdTime

���ܣ�ʵ��ͨ��ʱ��������յ�ת��

������ struct COMMONTIME *CT��ͨ��ʱ�Ľṹ��ָ�����
       struct MJDTIME *MJDT���������յĽṹ��ָ�����

�㷨��JD = INT[365.25y]+INT[30.6001(m+1)]+D+UT/24+1720981.5
      MJD = JD - 2400000.5

	  ���У�YΪ�꣬MΪ�£�DΪ�գ�UTΪ����ʱ����λΪСʱ����JDΪ������
	        ���M <= 2,��y = Y-1,m = M+12
			���M > 2,��y = Y,m = M
********************************************************/
bool CommonTimeToMjdTime(const struct COMMONTIME *CT, struct MJDTIME *MJDT)
{
	double JD= 0.0;
	double MJD= 0.0;
	double UT= 0.0;
	unsigned short y,m;
	y=m=0;

	if(CT->Year<1980 || CT->Year>2079 || CT->Month <0 || CT->Month>12 ||
		CT->Day<0 || CT->Day>31 || CT->Hour<0 || CT->Hour>24 ||
		CT->Minute<0 || CT->Minute>60 || CT->Second<0 || CT->Second>60)
	{
		printf("ͨ��ʱ�����д�������");
		return false;
	}
	//���Զ���һЩ�ݴ���

	if(CT->Month <=2)
	{
		y= CT->Year-1;
		m= CT->Month+12;
	}
	else
	{
		y= CT->Year;
		m= CT->Month;
	}
	UT= CT->Hour+(CT->Minute/60.0)+(CT->Second/3600);
	JD= (int)(365.25*y)+(int)(30.6001*(m+1))+CT->Day+
		(UT/24.0)+1720981.5;
	MJD= JD - 2400000.5;

	MJDT->Days= (int)MJD;
	MJDT->FracDay= MJD-(int)MJD;

	return true;
}

/********************************************************
MjdTimeToCommonTime

���ܣ�ʵ�ּ���������ͨ��ʱ��ת��

������struct MJDTIME *MJDT���������յĽṹ��ָ�����
	  struct COMMONTIME *CT��ͨ��ʱ�Ľṹ��ָ�����

�㷨��a = INT[JD + 0.5]
     b = a + 1537
	 c = INT[(b-122.1)/365.25]
	 d = INT[365.25*c]
	 e = INT[(b-d)/30.6001]
	 D = b - d - INT[30.6001*e] + FRAC[JD+0.5](С���գ�������ʱ���֡������Ϣ��
	 M = e - 1 - 12*INT[e/14]���£�
	 Y = c - 4715 - INT[(7+M)/10](��)
	 N = mod{INT[JD+0.5},7} (���ڼ���N=0������һ)

********************************************************/
bool MjdTimeToCommonTime(const struct MJDTIME *MJDT, struct COMMONTIME *CT)
{
	unsigned int a,b,c,d,e,M,Y;
	double D;
	a=b=c=d=e=M=Y=0;
	D=0.0;
	double MJD,JD;
	MJD=JD=0.0;
	unsigned short Hour,Min;
	double Sec;
	Hour=Min=0;
	Sec=0.0;

	MJD= MJDT->Days+ MJDT->FracDay;
	JD= MJD+2400000.5;

	if(JD<0)
	{
		printf("���������������,����");
		return false;
	}

	a= (int)(JD+ 0.5);
	b= a+1537;
	c= (int)((b-122.1)/365.25);
	d= (int)(365.25*c);
	e= (int)((b-d)/30.6001);
	D=  b - d - (int)(30.6001*e) + ((JD+0.5)-(int)(JD+0.5));
	M = e - 1 - 12*(int)(e/14.0);
    Y = c - 4715 - (int)((7+M)/10.0);

	CT->Year= Y;
	CT->Month= M;
	CT->Day= (int)D;
	Hour= (int)((D-(int)D)*24);
	Min= (int)((((D-(int)D)*24)-Hour)*60);
	Sec= (((((D-(int)D)*24)-Hour)*60)-Min)*60;
	CT->Hour= Hour;
	CT->Minute= Min;
	CT->Second= Sec;

	return true;
}

/********************************************************
MjdTimeToGpsTime

���ܣ�ʵ�ּ���������GPSʱ��ת��

������struct MJDTIME *MJDT���������յĽṹ��ָ�����
	 struct GPSTIME *GPST��GPSʱ�Ľṹ��ָ�����

�㷨��GPST.Week=(int)((MJD-44244)/7)
      GPST.Second=(MJD-44244-GPST.Week*7)*86400

********************************************************/
bool MjdTimeToGpsTime(const struct MJDTIME *MJDT, struct GPSTIME *GPST)
{
	double MJD;
	MJD=0.0;

	MJD=MJDT->Days+MJDT->FracDay;
	
	if(MJD<0)
	{
		printf("���������������,����");
		return false;
	}

	GPST->Week= (int)((MJD-44244)/7.0);
	GPST->SecOfWeek= (MJD-44244-(GPST->Week*7))*86400;

	return true;
}

/********************************************************
GpsTimeToMjdTime

���ܣ�ʵ��GPSʱ��������յ�ת��

������struct MJDTIME *MJDT���������յĽṹ��ָ�����
	 struct GPSTIME *GPST��GPSʱ�Ľṹ��ָ�����

�㷨��MJD= 44244+GPSWEEK*7+SECONDOFWEEK/86400

********************************************************/
bool GpsTimeToMjdTime(const struct GPSTIME *GPST, struct MJDTIME *MJDT)
{
	double MJD=0.0;

	MJD= 44244+(GPST->Week*7)+(GPST->SecOfWeek/86400.0);

	MJDT->Days= (int)MJD;
	MJDT->FracDay= MJD-(int)MJD;

	return true;
}

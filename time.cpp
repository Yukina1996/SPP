/********************************************************
时间转换相关计算的函数模块
函数功能：由通用时转化为简化儒略日/CommonTimeToMjdTime
		 由简化儒略日转化为通用时/MjdTimeToCommonTime
		 由简化儒略日转化为GPS时/MjdTimeToGpsTime
		 由GPS时转化为简化儒略日/GpsTimeToMjdTime

作者：王雅仪
日期：2017/4/28
********************************************************/
#include "stdafx.h"
#include "time.h"
#include "struct.h"

/********************************************************
CommonTimeToMjdTime

功能：实现通用时向简化儒略日的转化

参数： struct COMMONTIME *CT：通用时的结构体指针变量
       struct MJDTIME *MJDT：简化儒略日的结构体指针变量

算法：JD = INT[365.25y]+INT[30.6001(m+1)]+D+UT/24+1720981.5
      MJD = JD - 2400000.5

	  其中：Y为年，M为月，D为日，UT为世界时（单位为小时），JD为儒略日
	        如果M <= 2,则y = Y-1,m = M+12
			如果M > 2,则y = Y,m = M
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
		printf("通用时输入有错误，请检查");
		return false;
	}
	//可以多做一些容错处理

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

功能：实现简化儒略日向通用时的转化

参数：struct MJDTIME *MJDT：简化儒略日的结构体指针变量
	  struct COMMONTIME *CT：通用时的结构体指针变量

算法：a = INT[JD + 0.5]
     b = a + 1537
	 c = INT[(b-122.1)/365.25]
	 d = INT[365.25*c]
	 e = INT[(b-d)/30.6001]
	 D = b - d - INT[30.6001*e] + FRAC[JD+0.5](小数日，包含了时、分、秒的信息）
	 M = e - 1 - 12*INT[e/14]（月）
	 Y = c - 4715 - INT[(7+M)/10](年)
	 N = mod{INT[JD+0.5},7} (星期几，N=0，星期一)

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
		printf("简化儒略日输入出错,请检查");
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

功能：实现简化儒略日向GPS时的转化

参数：struct MJDTIME *MJDT：简化儒略日的结构体指针变量
	 struct GPSTIME *GPST：GPS时的结构体指针变量

算法：GPST.Week=(int)((MJD-44244)/7)
      GPST.Second=(MJD-44244-GPST.Week*7)*86400

********************************************************/
bool MjdTimeToGpsTime(const struct MJDTIME *MJDT, struct GPSTIME *GPST)
{
	double MJD;
	MJD=0.0;

	MJD=MJDT->Days+MJDT->FracDay;
	
	if(MJD<0)
	{
		printf("简化儒略日输入出错,请检查");
		return false;
	}

	GPST->Week= (int)((MJD-44244)/7.0);
	GPST->SecOfWeek= (MJD-44244-(GPST->Week*7))*86400;

	return true;
}

/********************************************************
GpsTimeToMjdTime

功能：实现GPS时向简化儒略日的转化

参数：struct MJDTIME *MJDT：简化儒略日的结构体指针变量
	 struct GPSTIME *GPST：GPS时的结构体指针变量

算法：MJD= 44244+GPSWEEK*7+SECONDOFWEEK/86400

********************************************************/
bool GpsTimeToMjdTime(const struct GPSTIME *GPST, struct MJDTIME *MJDT)
{
	double MJD=0.0;

	MJD= 44244+(GPST->Week*7)+(GPST->SecOfWeek/86400.0);

	MJDT->Days= (int)MJD;
	MJDT->FracDay= MJD-(int)MJD;

	return true;
}

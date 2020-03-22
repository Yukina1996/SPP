/*********************************************************************
加入差分改正数的单点定位模块：根据伪距观测值、星历计算卫星位置、RTCM的差分改正数

作者：王雅仪
时间：2017/6/8
*********************************************************************/
#include "stdafx.h"
#include"singlepointwithrtcm.h"
#include"rtcm.h"
#include"struct.h"
#include"matrix.h"
#include"space.h"
#include"solution.h"
#include"singlepoint.h"


/*********************************************************************
IsRTCMAvailable

功能：根据时间、卫星号、星历期号，判断这颗卫星的差分改正数是否可用

参数：const struct GPSTIME* Time：TNow
	short Prn：卫星号，判断是否读到了这颗卫星的差分改正数
	 double IODE：星历期号，差分改正数的期号必须和星历的期号一致才可用
	 const struct SATRTCM satrtcm[]：各颗卫星的改正数
	 double* prc：输出伪距改正值

返回值：true：该卫星有可用的差分改正数，且返回当前观测时刻的伪距改正值
	false：该卫星没有可用的差分改正数
*********************************************************************/
bool IsRTCMAvailable(const struct GPSTIME* Time, short Prn, double IODE, const struct SATRTCM satrtcm[], double* prc)
{
	double dt;

	//没有读到这颗卫星的差分改正数
	if(satrtcm[Prn-1].PRN != Prn || satrtcm[Prn-1].status==false)	
		return false;

	//星历期号和差分改正数的期号不一致
	if(fabs(IODE-satrtcm[Prn-1].IOD)>0.5)	
		return false;

	dt= fmod(Time->SecOfWeek, 3600) - satrtcm[Prn-1].T;

	*prc= satrtcm[Prn-1].RangeCorrection + satrtcm[Prn-1].RangeRateCorrection*dt;
	return true;
}

/*********************************************************************
calstawithrtcm

功能：根据观测值结构体，卫星星历，差分改正数进行单点定位算法，解算接收机的位置，
     钟差

参数：const struct RANGE *range：观测值结构体，包括了观测时间、观测的卫星数
	  和对应卫星的伪距
      const struct GPSEPHEM *ephem：卫星星历，用于解算卫星的位置、卫星钟差
	  struct CALSTAPOS *stapos：存放解算出来的接收机的位置、速度、钟差、钟速
	  struct CALSATPOS satpos[]：存放结算出来的各个卫星的位置、钟差
	  struct SATRTCM satrtcm[]：每颗卫星对应的差分改正数

返回值：0：正确结算，-1：未能解算
*********************************************************************/
int calstawithrtcm(const struct RANGE *range, const struct GPSEPHEM *ephem, const struct IONUTC *ion, struct CALSTAPOS *stapos, struct CALSATPOS satpos[], struct SATRTCM satrtcm[])
{
	double delta[4];						//最小二乘的待估参数
	double staXYZ[4];						//待求的测站的XYZ位置
	double staBLH[4];						//待求测站的BLH位置
	double L[NumOfChannel];					//L矩阵
	memset(L, 0, sizeof(L));
	double B[4*NumOfChannel];				//B矩阵
	memset(B, 0, sizeof(B));
	int counter;							//计数，记录符合的卫星数
	int i=0;								//循环变量i
	int j=0;								//循环变量j
	int k=0;								//循环变量k
	int m=0;								//循环变量m
	struct GPSTIME TSgps;					//信号发射时间（GPS系统时）
	double satclkerr;						//某卫星的卫星钟钟差
	double transmitime;						//信号传播时间
	double alpha;							//地球自转改正角度
	double eTrans[9];						//地球自转改正矩阵
	double satXYZ[3];						//中间变量，暂时存放卫星的位置
	double ele;								//卫星高度角
	double azi;								//卫星方位角
	double ionDelay=0;						//电离层延迟（米）
	double tropDelay=0;						//对流层延迟（米）
	double R;								//卫星到接收机的距离
	double BT[4*NumOfChannel];				//B矩阵的转置
	double BTP[4*NumOfChannel];				//BT*P
	double BTPB[16];						//BT*P*B
	double BTPBinv[16];						//BT*P*B的逆矩阵
	double BTPL[4];							//BT*P*L
	double V[NumOfChannel];					//残差矩阵
	double VT[NumOfChannel];				//残差矩阵的转置
	double BX[NumOfChannel];				//B*delta
	double VTV[1];							//VT*V
	double sigma;							//单位权中误差
	double PDOP;							//位置精度因子
	double DOP;								//精度因子
	bool IsDGPS;							//true:卫星差分改正数可用，false:卫星差分改正数不可用
	double weight[NumOfChannel];			//伪距观测值的权值
	double P[NumOfChannel*NumOfChannel];	//权矩阵
	double PRC;								
	short Prn;								//当前卫星号

	staXYZ[0]=stapos->staXYZ[0];						//测站X坐标初始值，上一历元解算的X坐标值
	staXYZ[1]=stapos->staXYZ[1];						//测站Y坐标初始值，上一历元解算的Y坐标值
	staXYZ[2]=stapos->staXYZ[2];						//测站Z坐标初始值，上一历元解算的Z坐标值
	staXYZ[3]=stapos->staClk;							//测站接收机钟差初值，上一历元解算的钟差

	delta[0]=0;
	delta[1]=0;
	delta[2]=0;
	delta[3]=0;

	do{
		i=0;
		j=0;
		staXYZ[0] = staXYZ[0]+delta[0];
		staXYZ[1] = staXYZ[1]+delta[1];
		staXYZ[2] = staXYZ[2]+delta[2];
		staXYZ[3] = staXYZ[3]+delta[3];

		while(i< range->NumofObs)
		{
			//计算卫星信号发射时间
			transmitime= range->Obs[i].Psr/C;

			if((range->ObsGPSTime.SecOfWeek-transmitime)<0)
			{
				TSgps.Week= range->ObsGPSTime.Week-1;
				TSgps.SecOfWeek= range->ObsGPSTime.SecOfWeek-transmitime+604800;
			}
			else
			{
				TSgps.Week= range->ObsGPSTime.Week;
				TSgps.SecOfWeek= range->ObsGPSTime.SecOfWeek-transmitime;
			} 

			satclkerr= calsatclk(&TSgps, &ephem[range->Obs[i].PRN-1], range->Obs[i].PRN);

			if(satclkerr==-1)
			{i++;continue;}													//该卫星星历过期，读取下一个观测的卫星 
			if(satclkerr==-2)
			{i++;continue;}													//该卫星没有星历，读取下一个观测的卫星 
			if(satclkerr==-3)												//该卫星非GPS卫星，不处理
			{i++;continue;}
			TSgps.SecOfWeek= TSgps.SecOfWeek - satclkerr;					//确定某卫星信号的发射时刻

			calsat(&ephem[range->Obs[i].PRN-1], &TSgps, range->Obs[i].PRN, &satpos[i]);
		
			//开始地球自转改正
			alpha= OMEGAedot*(range->ObsGPSTime.SecOfWeek - staXYZ[3]/C - TSgps.SecOfWeek);	//信号传播时地球转过的角度
			eTrans[0]=cos(alpha);
			eTrans[1]=sin(alpha);
			eTrans[2]=0;
			eTrans[3]=-sin(alpha);
			eTrans[4]=cos(alpha);
			eTrans[5]=0;
			eTrans[6]=0;
			eTrans[7]=0;
			eTrans[8]=1;

			matrix_multiply(3, 3, 3, 1, eTrans, satpos[i].satXYZ, satXYZ);
			satpos[i].satXYZ[0]= satXYZ[0];
			satpos[i].satXYZ[1]= satXYZ[1];
			satpos[i].satXYZ[2]= satXYZ[2];
			//结束地球自转改正
	
			if(sqrt(staXYZ[0]*staXYZ[0]+staXYZ[1]*staXYZ[1]+staXYZ[2]+staXYZ[2])>5000000
				&&sqrt(staXYZ[0]*staXYZ[0]+staXYZ[1]*staXYZ[1]+staXYZ[2]+staXYZ[2])<8000000)
			{
				calsatEA(staXYZ, satpos[i].satXYZ, &ele, &azi);
				satpos[i].eleAngle= ele;
				satpos[i].azimuth= azi;
				if((ele*180/PI) < 10)
				{i++;continue;}				//卫星高度角低于10°，读取下一个观测的卫星
				satpos[i].ionDelay= (klobuchar(&(range->ObsGPSTime), staXYZ, ele, azi, ion))*C;
				satpos[i].tropDelay= hopefield(staXYZ, ele);
			}

			R= sqrt((staXYZ[0]-satpos[i].satXYZ[0])*(staXYZ[0]-satpos[i].satXYZ[0])
				+(staXYZ[1]-satpos[i].satXYZ[1])*(staXYZ[1]-satpos[i].satXYZ[1])
				+(staXYZ[2]-satpos[i].satXYZ[2])*(staXYZ[2]-satpos[i].satXYZ[2]));
			B[j*4+0]= (staXYZ[0]-satpos[i].satXYZ[0])/R;
			B[j*4+1]= (staXYZ[1]-satpos[i].satXYZ[1])/R;
			B[j*4+2]= (staXYZ[2]-satpos[i].satXYZ[2])/R;
			B[j*4+3]= 1;

			Prn= range->Obs[i].PRN;			//当前观测值对应的卫星号
			if(IsDGPS=IsRTCMAvailable(&range->ObsGPSTime, range->Obs[i].PRN, ephem[Prn-1].IODE, satrtcm, &PRC))
			{
				L[j] = range->Obs[i].Psr - R - staXYZ[3] + C*satpos[i].satClk + PRC;
				weight[j]=1;
				j++;
			}

			i++;							//处理下一个观测数据
		}

		for(m=0;m<j;m++)
		{
			P[m*j+m]=weight[m];
		}

		if(j<=4)
			return -1;					//这组观测数据不可用，不能解算接收机位置
		else							//最小二乘解算
		{
			matrix_transfer(j, 4, B, BT);
			matrix_multiply(4, j, j, j, BT, P, BTP);
			matrix_multiply(4, j, j, 4, BTP, B, BTPB);
			matrix_inverse(4, BTPB, BTPBinv);
			matrix_multiply(4, j, j, 1, BTP, L, BTPL);
			matrix_multiply(4, 4, 4, 1, BTPBinv, BTPL, delta);
		}
		k++;
	}while(sqrt(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2])>0.001 && k<10);		//同时满足两个条件才循环

	//进行精度评估
	matrix_multiply(j, 4, 4, 1, B, delta, BX);
	matrix_minus(j, 1, j, 1, BX, L, V);
	matrix_transfer(j, 1, V, VT);
	matrix_multiply(1, j, j, 1, VT, V, VTV);
	sigma=sqrt(VTV[0]/(j-4));
	if(fabs(sigma)>10)	{return 1;}
	DOP= sqrt(BTPBinv[0]+BTPBinv[5]+BTPBinv[10]+BTPBinv[15]);
	PDOP= sqrt(BTPBinv[0]+BTPBinv[5]+BTPBinv[10]);

	staXYZ[0] += delta[0];
	staXYZ[1] += delta[1];
	staXYZ[2] += delta[2];
	staXYZ[3] += delta[3];

	stapos->staXYZ[0]= staXYZ[0];
	stapos->staXYZ[1]= staXYZ[1];
	stapos->staXYZ[2]= staXYZ[2];
	stapos->staClk= staXYZ[3];
	stapos->sigmaP= sigma;
	stapos->PDOP= PDOP;
	stapos->DOP_pos= DOP;

	XYZToBLH(stapos->staXYZ, stapos->staBLH, a_WGS84, e2_WGS84);
	return 0;
}


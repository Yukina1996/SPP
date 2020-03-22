/*********************************************************************
单点定位模块：根据伪距观测值计算测站的位置、速度、接收机钟差、钟速

作者：王雅仪
时间：2017/5/18
*********************************************************************/
#include "stdafx.h"
#include"singlepoint.h"
#include"struct.h"
#include"matrix.h"
#include"space.h"
#include"solution.h"

/*********************************************************************
calsta

功能：根据观测值结构体，卫星星历，进行单点定位算法，解算接收机的位置、速度，
     钟差、钟速

参数：const struct RANGE *range：观测值结构体，包括了观测时间、观测的卫星数
	  和对应卫星的伪距
      const struct GPSEPHEM *ephem：卫星星历，用于解算卫星的位置、卫星钟差
	  const struct IONUTC *ion：电离层误差改正模型结构体
	  struct CALSTAPOS *stapos：存放解算出来的接收机的位置、速度、钟差、钟速
	  struct CALSATPOS satpos[]：存放结算出来的各个卫星的位置、速度、钟差、钟速

返回值：0：正确结算，-1：未能解算
*********************************************************************/
int calsta(const struct RANGE *range, const struct GPSEPHEM *ephem, const struct IONUTC *ion, struct CALSTAPOS *stapos, struct CALSATPOS satpos[])
{
	double delta[4];						//最小二乘的待估参数
	double staXYZ[4];						//待求的测站的XYZ位置
	double staBLH[4];						//待求测站的BLH位置
	double L[NumOfChannel];					//L矩阵
	memset(L, 0, sizeof(L));
	double B[4 * NumOfChannel];				//B矩阵
	memset(B, 0, sizeof(B));
	int counter;							//计数，记录符合的卫星数
	int i = 0;								//循环变量i
	int j = 0;								//循环变量j
	int k = 0;								//循环变量k
	int m = 0;								//循环变量m
	struct GPSTIME TSgps;					//信号发射时间（GPS系统时）
	double satclkerr;						//某卫星的卫星钟钟差
	double transmitime;						//信号传播时间
	double alpha;							//地球自转改正角度
	double eTrans[9];						//地球自转改正矩阵
	double satXYZ[3];						//中间变量，暂时存放卫星的位置
	double ele;								//卫星高度角
	double azi;								//卫星方位角
	double ionDelay = 0;						//电离层延迟（米）
	double tropDelay = 0;						//对流层延迟（米）
	double R;								//卫星到接收机的距离
	double BT[4 * NumOfChannel];				//B矩阵的转置
	double BTB[16];							//BT*B
	double BTBinv[16];						//BT*B的转置矩阵
	double BTL[4];							//BT*L
	double V[NumOfChannel];					//残差矩阵
	double VT[NumOfChannel];				//残差矩阵的转置
	double BX[NumOfChannel];				//B*delta
	double VTV[1];							//VT*V
	double sigma;							//单位权中误差
	double PDOP;							//位置精度因子
	double DOP;								//精度因子

	staXYZ[0] = stapos->staXYZ[0];						//测站X坐标初始值，上一历元解算的X坐标值
	staXYZ[1] = stapos->staXYZ[1];						//测站Y坐标初始值，上一历元解算的Y坐标值
	staXYZ[2] = stapos->staXYZ[2];						//测站Z坐标初始值，上一历元解算的Z坐标值
	staXYZ[3] = stapos->staClk;							//测站接收机钟差初值，上一历元解算的钟差

	delta[0] = 0;
	delta[1] = 0;
	delta[2] = 0;
	delta[3] = 0;

	do {
		i = 0;
		j = 0;
		staXYZ[0] = staXYZ[0] + delta[0];
		staXYZ[1] = staXYZ[1] + delta[1];
		staXYZ[2] = staXYZ[2] + delta[2];
		staXYZ[3] = staXYZ[3] + delta[3];

		while (i < range->NumofObs)
		{
			//计算卫星信号发射时间
			transmitime = range->Obs[i].Psr / C;

			if ((range->ObsGPSTime.SecOfWeek - transmitime) < 0)
			{
				TSgps.Week = range->ObsGPSTime.Week - 1;
				TSgps.SecOfWeek = range->ObsGPSTime.SecOfWeek - transmitime + 604800;
			}
			else
			{
				TSgps.Week = range->ObsGPSTime.Week;
				TSgps.SecOfWeek = range->ObsGPSTime.SecOfWeek - transmitime;
			}

			satclkerr = calsatclk(&TSgps, &ephem[range->Obs[i].PRN - 1], range->Obs[i].PRN);

			if (satclkerr == -1)
			{
				i++; continue;
			}													//该卫星星历过期，读取下一个观测的卫星 
			if (satclkerr == -2)
			{
				i++; continue;
			}													//该卫星没有星历，读取下一个观测的卫星 
			if (satclkerr == -3)												//该卫星非GPS卫星，不处理
			{
				i++; continue;
			}
			TSgps.SecOfWeek = TSgps.SecOfWeek - satclkerr;					//确定某卫星信号的发射时刻

			calsat(&ephem[range->Obs[i].PRN - 1], &TSgps, range->Obs[i].PRN, &satpos[i]);

			//开始地球自转改正
			alpha = OMEGAedot*(range->ObsGPSTime.SecOfWeek - staXYZ[3] / C - TSgps.SecOfWeek);	//信号传播时地球转过的角度
			eTrans[0] = cos(alpha);
			eTrans[1] = sin(alpha);
			eTrans[2] = 0;
			eTrans[3] = -sin(alpha);
			eTrans[4] = cos(alpha);
			eTrans[5] = 0;
			eTrans[6] = 0;
			eTrans[7] = 0;
			eTrans[8] = 1;

			matrix_multiply(3, 3, 3, 1, eTrans, satpos[i].satXYZ, satXYZ);
			satpos[i].satXYZ[0] = satXYZ[0];
			satpos[i].satXYZ[1] = satXYZ[1];
			satpos[i].satXYZ[2] = satXYZ[2];
			//结束地球自转改正

			if (sqrt(staXYZ[0] * staXYZ[0] + staXYZ[1] * staXYZ[1] + staXYZ[2] + staXYZ[2]) > 5000000
				&& sqrt(staXYZ[0] * staXYZ[0] + staXYZ[1] * staXYZ[1] + staXYZ[2] + staXYZ[2]) < 8000000)
			{
				calsatEA(staXYZ, satpos[i].satXYZ, &ele, &azi);
				satpos[i].eleAngle = ele;
				satpos[i].azimuth = azi;
				if ((ele * 180 / PI) < 10)
				{
					i++; continue;
				}				//卫星高度角低于10°，读取下一个观测的卫星
				satpos[i].ionDelay = (klobuchar(&(range->ObsGPSTime), staXYZ, ele, azi, ion))*C;
				satpos[i].tropDelay = hopefield(staXYZ, ele);
			}

			R = sqrt((staXYZ[0] - satpos[i].satXYZ[0])*(staXYZ[0] - satpos[i].satXYZ[0])
				+ (staXYZ[1] - satpos[i].satXYZ[1])*(staXYZ[1] - satpos[i].satXYZ[1])
				+ (staXYZ[2] - satpos[i].satXYZ[2])*(staXYZ[2] - satpos[i].satXYZ[2]));
			B[j * 4 + 0] = (staXYZ[0] - satpos[i].satXYZ[0]) / R;
			B[j * 4 + 1] = (staXYZ[1] - satpos[i].satXYZ[1]) / R;
			B[j * 4 + 2] = (staXYZ[2] - satpos[i].satXYZ[2]) / R;
			B[j * 4 + 3] = 1;
			L[j] = range->Obs[i].Psr - R - staXYZ[3] + C*satpos[i].satClk - satpos[i].ionDelay - satpos[i].tropDelay;
			j++;
			i++;
		}

		if (j <= 4)
			return -1;					//这组观测数据不可用，不能解算接收机位置
		else							//最小二乘解算
		{
			matrix_transfer(j, 4, B, BT);
			matrix_multiply(4, j, j, 4, BT, B, BTB);
			matrix_inverse(4, BTB, BTBinv);
			matrix_multiply(4, j, j, 1, BT, L, BTL);
			matrix_multiply(4, 4, 4, 1, BTBinv, BTL, delta);
		}
		k++;
	} while (sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]) > 0.001 && k < 10);		//同时满足两个条件才循环

																										//进行精度评估
	matrix_multiply(j, 4, 4, 1, B, delta, BX);
	matrix_minus(j, 1, j, 1, BX, L, V);
	matrix_transfer(j, 1, V, VT);
	matrix_multiply(1, j, j, 1, VT, V, VTV);
	sigma = sqrt(VTV[0] / (j - 4));
	DOP = sqrt(BTBinv[0] + BTBinv[5] + BTBinv[10] + BTBinv[15]);
	PDOP = sqrt(BTBinv[0] + BTBinv[5] + BTBinv[10]);

	staXYZ[0] += delta[0];
	staXYZ[1] += delta[1];
	staXYZ[2] += delta[2];
	staXYZ[3] += delta[3];

	stapos->staXYZ[0] = staXYZ[0];
	stapos->staXYZ[1] = staXYZ[1];
	stapos->staXYZ[2] = staXYZ[2];
	stapos->staClk = staXYZ[3];
	stapos->sigmaP = sigma;
	stapos->PDOP = PDOP;
	stapos->DOP_pos = DOP;

	XYZToBLH(stapos->staXYZ, stapos->staBLH, a_WGS84, e2_WGS84);
	return 0;
}

/*********************************************************************
calsatclk

功能：根据输入的时间，对应的卫星星历，计算该卫星的卫星钟钟差

参数：const struct GPSTIME *time：输入时间
	 const struct GPSEPHEM *ephem：该卫星的星历
	 int PRN：某卫星的PRN号

返回值：double，卫星钟钟差;-1,星历过期;-2:没有该卫星的星历

说明：函数计算前要判断卫星是否有星历
*********************************************************************/
double calsatclk(const struct GPSTIME *time, const struct GPSEPHEM *ephem, int PRN)
{
	if(PRN>31)
		return -3;					//该卫星非GPS卫星
	if(ephem->PRN != PRN)			//该卫星没有星历
		return -2;
	double tc;						//输入时间和钟差参考时间的差
	double deltaTr;					//相对论效应改正
	double F;

	int i = 0;
	double Mk;						//平近点角(rad)
	double E = 0;					//偏近点角
	double Ek = 0;					//偏近点角
	double N0;						//平均角速度
	double tk;						//输入时间和星历参考时间的差
	double N;	

	double satclkerr;				//卫星钟钟差

	N0= sqrt(GM_WGS84/(ephem->A*ephem->A*ephem->A));
	tk= (time->Week - ephem->Week)*604800 + 
		(time->SecOfWeek - ephem->Toe);
	if(tk> 7200)					//判断星历是否过期
		return -1;
	N= N0 + ephem->deltaN;			//对平均运动角速度进行改正(rad/s)
	Mk= ephem->M0 + N*tk;			//rad

	do
	{
		E = Ek;
		Ek= Mk + ephem->ecc*sin(E);
		i++;
	}while((fabs(Ek-E)>THRESHOLD)||i<10);		//偏近点角(rad)

	tc =(time->Week - ephem->Week)*604800 + 
		(time->SecOfWeek - ephem->Toc);
	F= ((-2)*sqrt(1.0*GM_WGS84))/(C*C);
	deltaTr= F*ephem->ecc*sqrt(ephem->A)*sin(Ek);
	satclkerr= ephem->a0 + ephem->a1*tc + ephem->a2*tc*tc+deltaTr-ephem->Tgd;
	return satclkerr;
}

/*********************************************************************
calstavelocity

功能：根据获取的观测数据，上一步解出的观测卫星的解算结果（在信号发射时刻），
	和上一步解出的测站的解算结果，计算测站的速度。

参数：const struct RANGE *range：观测数据
	 const struct CALSATPOS satpos[]：观测卫星的解算结果
	 struct CALSTAPOS *stapos：测站的解算结果

返回值：-1：解算错误；0：正确解算
*********************************************************************/
int calstavelocity(const struct RANGE *range, const struct CALSATPOS satpos[], struct CALSTAPOS *stapos)
{
	int i=0;						//循环变量i
	int j=0;						//循环变量j
	int k=0;						//计数
	double B[4*NumOfChannel];		//设计矩阵
	double L[NumOfChannel];			//L-C矩阵
	double calXYZdot[4];			//待估的参数，测站速度和钟速
	double R;						//站星几何距离
	double Lambda1;					//L1信号的波长
	Lambda1= C/FreqL1;
	double BT[4*NumOfChannel];		//B的转置矩阵
	double BTP[4*NumOfChannel];		//BT*P
	double BTPB[16];				//BT*P*B
	double BTPBinv[16];				//BT*P*B的逆矩阵
	double BTPL[4];					//BT*P*L
	double BX[NumOfChannel];		//B*calXYZdot
	double V[NumOfChannel];			//残差矩阵
	double VT[NumOfChannel];		//残差矩阵的转置
	double VTPV[1];					//VT*P*V
	double VTP[NumOfChannel];		//VT*P
	double sigma;					//单位权中误差
	double ele;						//某卫星的高度角
	double azi;						//某卫星的方位角
	double P[NumOfChannel*NumOfChannel];
									//权阵
	memset(P, 0, sizeof(P));
	double reP[NumOfChannel];		//记录权值，以便形成矩阵

	while(i < range->NumofObs && j < range->NumofObs)
	{
		if(range->Obs[i].PRN == satpos[j].PRN)
		{
			if(satpos[j].eleAngle*180/PI < 10)
			{
				i++;					//读取下一个观测卫星的观测数据
				j++;					//读取下一个观测卫星的解算结果
				continue;
			}
			else
			{
				R=sqrt((satpos[j].satXYZ[0]-stapos->staXYZ[0])*(satpos[j].satXYZ[0]-stapos->staXYZ[0])+
					(satpos[j].satXYZ[1]-stapos->staXYZ[1])*(satpos[j].satXYZ[1]-stapos->staXYZ[1])+
					(satpos[j].satXYZ[2]-stapos->staXYZ[2])*(satpos[j].satXYZ[2]-stapos->staXYZ[2]));

				B[4*k+0]= (stapos->staXYZ[0]-satpos[j].satXYZ[0])/R;
				B[4*k+1]= (stapos->staXYZ[1]-satpos[j].satXYZ[1])/R;
				B[4*k+2]= (stapos->staXYZ[2]-satpos[j].satXYZ[2])/R;
				B[4*k+3]= 1;
				L[k]= -Lambda1*range->Obs[i].Dopp + C*satpos[j].satClkdot -
					((satpos[j].satXYZ[0]-stapos->staXYZ[0])*satpos[j].satXYZdot[0]+
					(satpos[j].satXYZ[1]-stapos->staXYZ[1])*satpos[j].satXYZdot[1]+
					(satpos[j].satXYZ[2]-stapos->staXYZ[2])*satpos[j].satXYZdot[2])/R;
				reP[k]= 1/cos(satpos[j].eleAngle);
				k++;
			}
		}
		else
		{
			i++;						//该卫星没有解算结果，读取下一个观测卫星的观测数据
			j++;continue;				//该卫星没有解算结果，读取下一卫星解算结果
		}
		i++;							//读取下一个观测卫星的观测数据
		j++;							//读取下一个观测卫星的解算结果
	}

	if(k<=4)
		return -1;
	for(i=0; i<k; i++)
	{
		P[i*k+i]=reP[i];
	}
	matrix_transfer(k, 4, B, BT);
	matrix_multiply(4, k, k, k, BT, P, BTP);
	matrix_multiply(4, k, k, 4, BTP, B, BTPB);
	matrix_inverse(4, BTPB, BTPBinv);
	matrix_multiply(4, k, k, 1, BTP, L, BTPL);
	matrix_multiply(4, 4, 4, 1, BTPBinv, BTPL, calXYZdot);

	stapos->staXYZdot[0]= calXYZdot[0];
	stapos->staXYZdot[1]= calXYZdot[1];
	stapos->staXYZdot[2]= calXYZdot[2];
	stapos->staClkdot= calXYZdot[3];
	
	//精度评估
	matrix_multiply(k, 4, 4, 1, B, calXYZdot, BX);
	matrix_minus(k, 1, k, 1, L, BX, V);
	matrix_transfer(k, 1, V, VT);
	matrix_multiply(1, k, k, k, VT, P, VTP);
	matrix_multiply(1, k, k, 1, VTP, V, VTPV);
	sigma= sqrt(VTPV[0]/(k-4));

	stapos->sigmaV=sigma;
	stapos->DOP_v= sqrt(BTPBinv[0]+BTPBinv[5]+BTPBinv[10]+BTPBinv[15]);

	return 0;

}
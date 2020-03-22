/*********************************************************************
星历模块：包括根据星历计算卫星的位置、速度和卫星钟差、钟速
		 根据星历计算电离层、对流层误差改正

作者：王雅仪
时间：2017/5/15
*********************************************************************/
#include "stdafx.h"
#include"solution.h"
#include"struct.h"
#include"matrix.h"
#include"space.h"

/*********************************************************************
calsat

功能：判断该卫星是否有星历、星历是否过期
	根据星历计算卫星的位置、速度和钟差信息
	输出到对应卫星的结果结构体里

参数：const struct GPSEPHEM *ephem：该卫星对应的星历结构体
      const struct GPSTIME *time：时间，卫星在该时间下的结果
	  int PRN：决定是关于哪颗卫星的结果
	  struct CALSATPOS satpos：卫星解算结果结构体

返回值：如果解算出了相应卫星的结果，输出该卫星号
*********************************************************************/
int calsat(const struct GPSEPHEM * ephem, const struct GPSTIME * time, int PRN, struct CALSATPOS * satpos)
{
	if (PRN > 31)
		return -1;
	if (ephem->PRN != PRN || PRN == 0)				//没有该卫星的星历
		return -1;
	else
	{
		//计算卫星在地球地固坐标系中的位置
		double N0;						//平均角速度
		double tk;						//输入时间和星历参考时间的差
		double N;
		double Mk;						//平近点角(rad)
		double E = 0;					//偏近点角
		double Ek = 0;					//偏近点角
		int i = 0;
		double Vk;						//真近点角(rad)
		double Phik;					//升交角距(rad)
		double Miuk;					//经过改正的升交角距(rad)
		double rk;						//卫星向径(m)
		double ik;						//经过改正的倾角
		double xk;						//卫星在轨道坐标系的X坐标
		double yk;						//卫星在轨道坐标系的Y坐标
		double Omegak;					//改正后的升交点经度

		double Ekdot;					//偏近点角的变化速度
		double Phikdot;					//升交角距的变化速度
		double Miukdot;					//改进后的升交角距的变化速度
		double rkdot;					//卫星向径的变化速度
		double ikdot;					//轨道倾角的变化速度
		double Omegakdot;				//升交点经度的变化速度
		double Rdot[12];				//计算卫星运动速度的转移矩阵
		double xkdot;					//卫星在轨道坐标系的坐标变化速度
		double ykdot;					//卫星在轨道坐标系的坐标变化速度
		double XYdot[4];

		double tc;						//输入时间和钟差参考时间的差
		double deltaTr;					//相对论效应改正
		double F;
		double deltaTrdot;				//相对论效应改正的导数

		N0 = sqrt(GM_WGS84 / (ephem->A*ephem->A*ephem->A));
		tk = (time->Week - ephem->Week) * 604800 +
			(time->SecOfWeek - ephem->Toe);
		if (tk > 7200)					//判断星历是否过期
			return 0;
		N = N0 + ephem->deltaN;			//对平均运动角速度进行改正(rad/s)
		Mk = ephem->M0 + N*tk;			//rad

		do
		{
			E = Ek;
			Ek = Mk + ephem->ecc*sin(E);
			i++;
		} while ((fabs(Ek - E) > THRESHOLD) || i < 10);		//偏近点角(rad)

		Vk = atan2((sqrt(1 - ephem->ecc*ephem->ecc))*sin(Ek),
			cos(Ek) - ephem->ecc);					//真近点角(rad)

		Phik = Vk + ephem->omega;					//升交角距(rad)
		Miuk = Phik + ephem->cus*sin(2 * Phik) + ephem->cuc*cos(2 * Phik);

		rk = ephem->A*(1 - ephem->ecc*cos(Ek)) +
			ephem->crs*sin(2 * Phik) + ephem->crc*cos(2 * Phik);

		ik = ephem->I0 + ephem->cis*sin(2 * Phik) + ephem->cic*cos(2 * Phik) +
			ephem->RofI*tk;

		xk = rk*cos(Miuk);
		yk = rk*sin(Miuk);

		Omegak = ephem->omega0 + (ephem->RofOmega0 - OMEGAedot)*tk -
			OMEGAedot*ephem->Toe;

		satpos->satXYZ[0] = xk*cos(Omegak) - yk*cos(ik)*sin(Omegak);
		satpos->satXYZ[1] = xk*sin(Omegak) + yk*cos(ik)*cos(Omegak);
		satpos->satXYZ[2] = yk*sin(ik);
		//结束计算卫星在地球地固坐标系中的位置

		//计算卫星在地球地固坐标系中的速度
		Ekdot = N / (1 - ephem->ecc*cos(Ek));
		Phikdot = (sqrt(1 - ephem->ecc*ephem->ecc)*Ekdot) /
			(1 - ephem->ecc*cos(Ek));
		Miukdot = 2 * (ephem->cus*cos(2 * Phik) - ephem->cuc*sin(2 * Phik))*Phikdot +
			Phikdot;
		rkdot = ephem->A*ephem->ecc*sin(Ek)*Ekdot +
			2 * (ephem->crs*cos(2 * Phik) - ephem->crc*sin(2 * Phik))*Phikdot;
		ikdot = ephem->RofI + 2 * (ephem->cis*cos(2 * Phik) - ephem->cic*sin(2 * Phik))*Phikdot;
		Omegakdot = ephem->RofOmega0 - OMEGAedot;

		Rdot[0] = cos(Omegak);
		Rdot[1] = -sin(Omegak)*cos(ik);
		Rdot[2] = -(xk*sin(Omegak) + yk*cos(Omegak)*cos(ik));
		Rdot[3] = yk*sin(Omegak)*sin(ik);
		Rdot[4] = sin(Omegak);
		Rdot[5] = cos(Omegak)*cos(ik);
		Rdot[6] = xk*cos(Omegak) - yk*sin(Omegak)*cos(ik);
		Rdot[7] = yk*cos(Omegak)*sin(ik);
		Rdot[8] = 0;
		Rdot[9] = sin(ik);
		Rdot[10] = 0;
		Rdot[11] = yk*cos(ik);

		xkdot = rkdot*cos(Miuk) - rk*Miukdot*sin(Miuk);
		ykdot = rkdot*sin(Miuk) + rk*Miukdot*cos(Miuk);

		XYdot[0] = xkdot;
		XYdot[1] = ykdot;
		XYdot[2] = Omegakdot;
		XYdot[3] = ikdot;

		matrix_multiply(3, 4, 4, 1, Rdot, XYdot, satpos->satXYZdot);
		//结束计算卫星在地球地固坐标系中的速度

		//计算卫星的钟差、钟速
		tc = (time->Week - ephem->Week) * 604800 +
			(time->SecOfWeek - ephem->Toc);
		F = ((-2)*sqrt(1.0*GM_WGS84)) / (C*C);
		deltaTr = F*ephem->ecc*sqrt(ephem->A)*sin(Ek);
		satpos->satClk = ephem->a0 + ephem->a1*tc +
			ephem->a2*tc*tc + deltaTr - ephem->Tgd;

		deltaTrdot = F*ephem->ecc*sqrt(ephem->A)*cos(Ek)*Ekdot;
		satpos->satClkdot = ephem->a1 + 2 * ephem->a2*tc + deltaTrdot;
		//结束计算卫星的钟差、钟速
	}
	satpos->PRN = PRN;
	return PRN;
}

/*********************************************************************
calsatSA

功能：根据测站和卫星在地球地固坐标系中的XYZ坐标，计算在测站站心坐标系中卫星
	的高度角和方位角。

参数：const double staXYZ[3]：测站坐标
      const double satXYZ[3]：卫星坐标
	  double *eleAngle：指向实参高度角的地址，高度角单位为弧度，取值范围-PI~PI
	  double *azimuth：指向实参方位角的地址，方位角单位为弧度，取值范围为0~2PI

返回值：0，正确解算
*********************************************************************/
int calsatEA(const double staXYZ[3], const double satXYZ[3], double *eleAngle, double *azimuth)
{
	double staBLH[3];								//测站的大地经纬度和高程
	double Trans[9];								//声明转置矩阵
	double satsta[3];								//测站与卫星的连线
	double satxyz[3];								//卫星在站心坐标系的坐标
	XYZToBLH(staXYZ, staBLH, a_WGS84, e2_WGS84);

	staBLH[0]=staBLH[0]*PI/180.0;					//将纬度的单位转为弧度rad
	staBLH[1]=staBLH[1]*PI/180.0;					//将经度的单位转为弧度rad

	Trans[0]= -sin(staBLH[0])*cos(staBLH[1]);
	Trans[1]= -sin(staBLH[0])*sin(staBLH[1]);
	Trans[2]= cos(staBLH[0]);
	Trans[3]= -sin(staBLH[1]);
	Trans[4]= cos(staBLH[1]);
	Trans[5]= 0;
	Trans[6]= cos(staBLH[0])*cos(staBLH[1]);
	Trans[7]= cos(staBLH[0])*sin(staBLH[1]);
	Trans[8]= sin(staBLH[0]);

	satsta[0]= satXYZ[0]-staXYZ[0];
	satsta[1]= satXYZ[1]-staXYZ[1];
	satsta[2]= satXYZ[2]-staXYZ[2];

	matrix_multiply(3, 3, 3, 1,Trans, satsta, satxyz);

	*eleAngle= asin(satxyz[2]/
		(sqrt(satxyz[0]*satxyz[0]+satxyz[1]*satxyz[1]+satxyz[2]*satxyz[2])));		//高度角，rad
	*azimuth= atan2(satxyz[1],satxyz[0]);											//方位角，rad
	if(*azimuth<0)
		*azimuth=(*azimuth)+2*PI;

	return 0;
}

/*********************************************************************
klobuchar

功能：根据测站的位置以及观测时刻、电离层改正模型结构体
	，解算电离层误差

参数：const struct GPSTIME *time：观测时刻
      double staXYZ[3]：用户接收机的坐标
	  double eleAngle：高度角，高度角单位为弧度，取值范围-PI~PI
	  double azimuth：方位角，方位角单位为弧度，取值范围为0~2PI
	  struct GPSEPHEM *ephem：某颗卫星的星历

返回值：double 电离层的改正值
*********************************************************************/
double klobuchar(const struct GPSTIME *time, const double staXYZ[3], double eleAngle, double azimuth, const struct IONUTC *ion)
{
	double staBLH[3];								//用户接收机的经纬度和高程
	double EA;										//计算测站和穿刺点的连线与地心的夹角
	double IPPLat;									//穿刺点的纬度
	double IPPLon;									//穿刺点的经度
	double IPPmagLat;								//穿测点的地磁纬度
	double IPPmagLon;								//穿刺点的地磁经度
	double IPPt;									//穿刺点的当地时间
	double Ai;										//电离层延迟的幅值
	double Pi;										//电离层延迟的周期
	double Xi;										//电离层延迟的相位
	double SF;										//计算倾斜因子
	double IonDelay;								//电离层延迟（秒）
	XYZToBLH(staXYZ, staBLH, a_WGS84, e2_WGS84);	//经纬度单位为度

	staBLH[0]= staBLH[0]/180.0;						//将纬度单位转化为半周
	staBLH[1]= staBLH[1]/180.0;						//将经度单位转化为半周
	eleAngle= eleAngle/PI;							//高度角单位转化为半周
	azimuth=  azimuth/PI;							//方位角单位转化为半周

	EA= (0.0137/(eleAngle+0.11))-0.022;					//半周
	IPPLat= staBLH[0]+EA*cos(azimuth*PI);				//计算穿刺点的纬度
	if(IPPLat>0.416)
		IPPLat= 0.146;
	if(IPPLat<-0.416)
		IPPLat=-0.416;
	IPPLon= staBLH[1]+EA*sin(azimuth*PI)/cos(IPPLat*PI);	//计算穿刺点的经度
	IPPmagLat= IPPLat+0.064*cos((IPPLon-1.617)*PI);		
	IPPt= 43200*IPPLon + time->SecOfWeek;
	IPPt = fmod(IPPt, 86400.0);
	if(IPPt >= 86400)
		IPPt -= 86400;
	if(IPPt < 0)
		IPPt += 86400;

	Ai= ion->a0 + ion->a1*IPPmagLat + ion->a2*IPPmagLat*IPPmagLat
		+ ion->a3*IPPmagLat*IPPmagLat*IPPmagLat;
	if(Ai<0)
		Ai=0;
	Pi= ion->b0 + ion->b1*IPPmagLat + ion->b2*IPPmagLat*IPPmagLat
		+ ion->b3*IPPmagLat*IPPmagLat*IPPmagLat;
	if(Pi<72000)
		Pi=72000;
	Xi= 2*PI*(IPPt-50400)/Pi;
	SF= 1.0+16.0*(0.53-eleAngle)*(0.53-eleAngle)*(0.53-eleAngle);

	if(fabs(Xi)<1.57)
		IonDelay=(0.000000005+Ai*(1-(Xi*Xi/2.0)+(Xi*Xi*Xi*Xi/24.0)))*SF;
	if(fabs(Xi)>=1.57)
		IonDelay=0.000000005*SF;
	
	return IonDelay;
}

/*********************************************************************
hopefield

功能：根据测站位置、气象元素、卫星高度角计算对流层误差改正值

参数：const double staXYZ[3]：测站的位置
	const double eleAngle：卫星高度角，单位为弧度

返回值：double 对流层的模型改正值
*********************************************************************/
double hopefield(const double staXYZ[3], double eleAngle)
{
	double Kd;
	double Kw;
	double hd;
	double hw;
	double e;
	double T;
	double p;
	double RH;
	double staBLH[3];					//测站的纬度、经度、高程
	double TropDelay;					//对流层延迟（米）
	eleAngle=eleAngle*180/PI;			//将高度角单位由弧度转化为度
	XYZToBLH(staXYZ, staBLH, a_WGS84, e2_WGS84);

	if((1-0.0000226*staBLH[2])<0)
		return 0;
	else
	{
		RH= 0.5*exp(-0.0006396*staBLH[2]);
		p= 1013.25*pow(1-0.0000226*staBLH[2],5.225);
		T= (15+273.16)-0.0065*staBLH[2];
		e= RH*exp(-37.2465+0.213166*T-0.000256908*T*T);
		hw= 11000;
		hd= 40136+148.72*15;
		Kw= 155.2*0.0000001*4810*e*(hw-staBLH[2])/(T*T);
		Kd= 155.2*0.0000001*p*(hd-staBLH[2])/T;

		TropDelay= Kd/sin((sqrt(eleAngle*eleAngle+6.25))*PI/180.0)+
			Kw/sin((sqrt(eleAngle*eleAngle+2.25))*PI/180.0);

		return TropDelay;
	}
}
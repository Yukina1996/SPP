/********************************************************
空间坐标系转换的相关计算的函数模块
函数功能：大地坐标到笛卡尔坐标的转换/BLHToXYZ
         笛卡尔坐标到大地坐标的转换/XYZToBLH

作者：王雅仪
日期：2017/4/28
********************************************************/
#include "stdafx.h"
#include "space.h"

#define PI 3.1415926535897932384626433832795

/********************************************************
BLHToXYZ

功能：实现大地坐标到笛卡尔坐标的转换

参数： BLH[]：存放大地坐标的数组
       BLH[0]=B/纬度,BLH[1]=L/经度,BLH[2]=H/高程
	   （B,L单位为度，H单位为米）
       XYZ[]：存放笛卡尔坐标的数组
	   XYZ[0]=X, XYZ[1]=Y, XYZ[2]=Z
	   （X,Y,Z单位为米）
	   a是对应坐标系的地球长半轴
	   e2是对应坐标系的第一偏心率的平方

算法：X=(N+H)cosBcosL
     Y=(N+H)cosBsinL
	 Z=[N(1-e^2)+H]sinB

	 其中：N为卯酉圈的半径,
	 N=a/sqrt(1-e^2*(sinB)^2)
	 e是第一偏心率
********************************************************/
bool BLHToXYZ(const double BLH[], double XYZ[], const double a, const double e2)
{
	double N=0.0;
	double B,L,H,X,Y,Z;
	//将纬度B，经度L的单位由度转换为弧度
	B=PI*BLH[0]/180;
	L=PI*BLH[1]/180;
	H=BLH[2];

	N= a/sqrt(1-e2*(sin(B)*sin(B)));
	X= (N+H)*cos(B)*cos(L);
	Y= (N+H)*cos(B)*sin(L);
	Z= (N*(1-e2)+H)*sin(B);

    XYZ[0]=X;
	XYZ[1]=Y;
	XYZ[2]=Z;

	return true;
}

/********************************************************
XYZToBLH

功能：实现笛卡尔坐标到大地坐标的转换

参数： BLH[]：存放大地坐标的数组
       BLH[0]=B/纬度,BLH[1]=L/经度,BLH[2]=H/高程
	   （B,L单位为度，H单位为米）
       XYZ[]：存放笛卡尔坐标的数组
	   XYZ[0]=X, XYZ[1]=Y, XYZ[2]=Z
	     （X,Y,Z单位为米）
	   a是对应坐标系的地球长半轴
	   e2是对应坐标系的第一偏心率的平方

算法：L=atan2(Y/X)
      B=atan((Z+△Z)/sqrt(X^2+Y^2))
	  H=sqrt(X^2+Y^2+(Z+△Z)^2)-N

	  其中：N=a/sqrt(1-e^2*(sinB)^2)
	  △Z=N*e^2*sinB
	  需要迭代运算，初始时，令△Z=e^2*Z
********************************************************/
bool XYZToBLH(const double XYZ[], double BLH[], const double a, const double e2)
{
	double N=0.0;
	double X,Y,Z,B,L,H;
	X=XYZ[0];
	Y=XYZ[1];
	Z=XYZ[2];
	double deltaZ=0.0;
	//初始化deltaZ
	deltaZ=e2*Z;
	//中间变量temp
	double temp1=0.0;
	double temp2=0.0;

	L= atan2(Y,X);

	//迭代法1：用B来决定是否继续迭代
	/*do{
		B=atan((Z+deltaZ)/sqrt(X*X+Y*Y));
		temp1= (Z+deltaZ)/sqrt(X*X+Y*Y+(Z+deltaZ)*(Z+deltaZ));
		temp2= sin(B);
	    N= a/(sqrt(1-e2*sin(B)*sin(B)));
		deltaZ= N*e2*sin(B);
	}while(fabs(temp1-temp2)>0.00001);*/

	//迭代法2:用deltaZ来决定是否继续迭代
	do{
		deltaZ=temp1;
		B=atan((Z+deltaZ)/sqrt(X*X+Y*Y));
		N= a/(sqrt(1-e2*sin(B)*sin(B)));
		temp1= N*e2*sin(B);
	}while(fabs(temp1-deltaZ)>0.00000001);

	H= sqrt(X*X+Y*Y+(Z+deltaZ)*(Z+deltaZ))-N;

	BLH[0]=180*B/PI;
	BLH[1]=180*L/PI;
	BLH[2]=H;

	return true;
}

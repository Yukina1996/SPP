/********************************************************
�ռ�����ϵת������ؼ���ĺ���ģ��
�������ܣ�������굽�ѿ��������ת��/BLHToXYZ
         �ѿ������굽��������ת��/XYZToBLH

���ߣ�������
���ڣ�2017/4/28
********************************************************/
#include "stdafx.h"
#include "space.h"

#define PI 3.1415926535897932384626433832795

/********************************************************
BLHToXYZ

���ܣ�ʵ�ִ�����굽�ѿ��������ת��

������ BLH[]����Ŵ�����������
       BLH[0]=B/γ��,BLH[1]=L/����,BLH[2]=H/�߳�
	   ��B,L��λΪ�ȣ�H��λΪ�ף�
       XYZ[]����ŵѿ������������
	   XYZ[0]=X, XYZ[1]=Y, XYZ[2]=Z
	   ��X,Y,Z��λΪ�ף�
	   a�Ƕ�Ӧ����ϵ�ĵ��򳤰���
	   e2�Ƕ�Ӧ����ϵ�ĵ�һƫ���ʵ�ƽ��

�㷨��X=(N+H)cosBcosL
     Y=(N+H)cosBsinL
	 Z=[N(1-e^2)+H]sinB

	 ���У�NΪî��Ȧ�İ뾶,
	 N=a/sqrt(1-e^2*(sinB)^2)
	 e�ǵ�һƫ����
********************************************************/
bool BLHToXYZ(const double BLH[], double XYZ[], const double a, const double e2)
{
	double N=0.0;
	double B,L,H,X,Y,Z;
	//��γ��B������L�ĵ�λ�ɶ�ת��Ϊ����
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

���ܣ�ʵ�ֵѿ������굽��������ת��

������ BLH[]����Ŵ�����������
       BLH[0]=B/γ��,BLH[1]=L/����,BLH[2]=H/�߳�
	   ��B,L��λΪ�ȣ�H��λΪ�ף�
       XYZ[]����ŵѿ������������
	   XYZ[0]=X, XYZ[1]=Y, XYZ[2]=Z
	     ��X,Y,Z��λΪ�ף�
	   a�Ƕ�Ӧ����ϵ�ĵ��򳤰���
	   e2�Ƕ�Ӧ����ϵ�ĵ�һƫ���ʵ�ƽ��

�㷨��L=atan2(Y/X)
      B=atan((Z+��Z)/sqrt(X^2+Y^2))
	  H=sqrt(X^2+Y^2+(Z+��Z)^2)-N

	  ���У�N=a/sqrt(1-e^2*(sinB)^2)
	  ��Z=N*e^2*sinB
	  ��Ҫ�������㣬��ʼʱ�����Z=e^2*Z
********************************************************/
bool XYZToBLH(const double XYZ[], double BLH[], const double a, const double e2)
{
	double N=0.0;
	double X,Y,Z,B,L,H;
	X=XYZ[0];
	Y=XYZ[1];
	Z=XYZ[2];
	double deltaZ=0.0;
	//��ʼ��deltaZ
	deltaZ=e2*Z;
	//�м����temp
	double temp1=0.0;
	double temp2=0.0;

	L= atan2(Y,X);

	//������1����B�������Ƿ��������
	/*do{
		B=atan((Z+deltaZ)/sqrt(X*X+Y*Y));
		temp1= (Z+deltaZ)/sqrt(X*X+Y*Y+(Z+deltaZ)*(Z+deltaZ));
		temp2= sin(B);
	    N= a/(sqrt(1-e2*sin(B)*sin(B)));
		deltaZ= N*e2*sin(B);
	}while(fabs(temp1-temp2)>0.00001);*/

	//������2:��deltaZ�������Ƿ��������
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

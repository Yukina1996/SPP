/*********************************************************************
�����ָ������ĵ��㶨λģ�飺����α��۲�ֵ��������������λ�á�RTCM�Ĳ�ָ�����

���ߣ�������
ʱ�䣺2017/6/8
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

���ܣ�����ʱ�䡢���Ǻš������ںţ��ж�������ǵĲ�ָ������Ƿ����

������const struct GPSTIME* Time��TNow
	short Prn�����Ǻţ��ж��Ƿ������������ǵĲ�ָ�����
	 double IODE�������ںţ���ָ��������ںű�����������ں�һ�²ſ���
	 const struct SATRTCM satrtcm[]���������ǵĸ�����
	 double* prc�����α�����ֵ

����ֵ��true���������п��õĲ�ָ��������ҷ��ص�ǰ�۲�ʱ�̵�α�����ֵ
	false��������û�п��õĲ�ָ�����
*********************************************************************/
bool IsRTCMAvailable(const struct GPSTIME* Time, short Prn, double IODE, const struct SATRTCM satrtcm[], double* prc)
{
	double dt;

	//û�ж���������ǵĲ�ָ�����
	if(satrtcm[Prn-1].PRN != Prn || satrtcm[Prn-1].status==false)	
		return false;

	//�����ںźͲ�ָ��������ںŲ�һ��
	if(fabs(IODE-satrtcm[Prn-1].IOD)>0.5)	
		return false;

	dt= fmod(Time->SecOfWeek, 3600) - satrtcm[Prn-1].T;

	*prc= satrtcm[Prn-1].RangeCorrection + satrtcm[Prn-1].RangeRateCorrection*dt;
	return true;
}

/*********************************************************************
calstawithrtcm

���ܣ����ݹ۲�ֵ�ṹ�壬������������ָ��������е��㶨λ�㷨��������ջ���λ�ã�
     �Ӳ�

������const struct RANGE *range���۲�ֵ�ṹ�壬�����˹۲�ʱ�䡢�۲��������
	  �Ͷ�Ӧ���ǵ�α��
      const struct GPSEPHEM *ephem���������������ڽ������ǵ�λ�á������Ӳ�
	  struct CALSTAPOS *stapos����Ž�������Ľ��ջ���λ�á��ٶȡ��Ӳ����
	  struct CALSATPOS satpos[]����Ž�������ĸ������ǵ�λ�á��Ӳ�
	  struct SATRTCM satrtcm[]��ÿ�����Ƕ�Ӧ�Ĳ�ָ�����

����ֵ��0����ȷ���㣬-1��δ�ܽ���
*********************************************************************/
int calstawithrtcm(const struct RANGE *range, const struct GPSEPHEM *ephem, const struct IONUTC *ion, struct CALSTAPOS *stapos, struct CALSATPOS satpos[], struct SATRTCM satrtcm[])
{
	double delta[4];						//��С���˵Ĵ�������
	double staXYZ[4];						//����Ĳ�վ��XYZλ��
	double staBLH[4];						//�����վ��BLHλ��
	double L[NumOfChannel];					//L����
	memset(L, 0, sizeof(L));
	double B[4*NumOfChannel];				//B����
	memset(B, 0, sizeof(B));
	int counter;							//��������¼���ϵ�������
	int i=0;								//ѭ������i
	int j=0;								//ѭ������j
	int k=0;								//ѭ������k
	int m=0;								//ѭ������m
	struct GPSTIME TSgps;					//�źŷ���ʱ�䣨GPSϵͳʱ��
	double satclkerr;						//ĳ���ǵ��������Ӳ�
	double transmitime;						//�źŴ���ʱ��
	double alpha;							//������ת�����Ƕ�
	double eTrans[9];						//������ת��������
	double satXYZ[3];						//�м��������ʱ������ǵ�λ��
	double ele;								//���Ǹ߶Ƚ�
	double azi;								//���Ƿ�λ��
	double ionDelay=0;						//������ӳ٣��ף�
	double tropDelay=0;						//�������ӳ٣��ף�
	double R;								//���ǵ����ջ��ľ���
	double BT[4*NumOfChannel];				//B�����ת��
	double BTP[4*NumOfChannel];				//BT*P
	double BTPB[16];						//BT*P*B
	double BTPBinv[16];						//BT*P*B�������
	double BTPL[4];							//BT*P*L
	double V[NumOfChannel];					//�в����
	double VT[NumOfChannel];				//�в�����ת��
	double BX[NumOfChannel];				//B*delta
	double VTV[1];							//VT*V
	double sigma;							//��λȨ�����
	double PDOP;							//λ�þ�������
	double DOP;								//��������
	bool IsDGPS;							//true:���ǲ�ָ��������ã�false:���ǲ�ָ�����������
	double weight[NumOfChannel];			//α��۲�ֵ��Ȩֵ
	double P[NumOfChannel*NumOfChannel];	//Ȩ����
	double PRC;								
	short Prn;								//��ǰ���Ǻ�

	staXYZ[0]=stapos->staXYZ[0];						//��վX�����ʼֵ����һ��Ԫ�����X����ֵ
	staXYZ[1]=stapos->staXYZ[1];						//��վY�����ʼֵ����һ��Ԫ�����Y����ֵ
	staXYZ[2]=stapos->staXYZ[2];						//��վZ�����ʼֵ����һ��Ԫ�����Z����ֵ
	staXYZ[3]=stapos->staClk;							//��վ���ջ��Ӳ��ֵ����һ��Ԫ������Ӳ�

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
			//���������źŷ���ʱ��
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
			{i++;continue;}													//�������������ڣ���ȡ��һ���۲������ 
			if(satclkerr==-2)
			{i++;continue;}													//������û����������ȡ��һ���۲������ 
			if(satclkerr==-3)												//�����Ƿ�GPS���ǣ�������
			{i++;continue;}
			TSgps.SecOfWeek= TSgps.SecOfWeek - satclkerr;					//ȷ��ĳ�����źŵķ���ʱ��

			calsat(&ephem[range->Obs[i].PRN-1], &TSgps, range->Obs[i].PRN, &satpos[i]);
		
			//��ʼ������ת����
			alpha= OMEGAedot*(range->ObsGPSTime.SecOfWeek - staXYZ[3]/C - TSgps.SecOfWeek);	//�źŴ���ʱ����ת���ĽǶ�
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
			//����������ת����
	
			if(sqrt(staXYZ[0]*staXYZ[0]+staXYZ[1]*staXYZ[1]+staXYZ[2]+staXYZ[2])>5000000
				&&sqrt(staXYZ[0]*staXYZ[0]+staXYZ[1]*staXYZ[1]+staXYZ[2]+staXYZ[2])<8000000)
			{
				calsatEA(staXYZ, satpos[i].satXYZ, &ele, &azi);
				satpos[i].eleAngle= ele;
				satpos[i].azimuth= azi;
				if((ele*180/PI) < 10)
				{i++;continue;}				//���Ǹ߶Ƚǵ���10�㣬��ȡ��һ���۲������
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

			Prn= range->Obs[i].PRN;			//��ǰ�۲�ֵ��Ӧ�����Ǻ�
			if(IsDGPS=IsRTCMAvailable(&range->ObsGPSTime, range->Obs[i].PRN, ephem[Prn-1].IODE, satrtcm, &PRC))
			{
				L[j] = range->Obs[i].Psr - R - staXYZ[3] + C*satpos[i].satClk + PRC;
				weight[j]=1;
				j++;
			}

			i++;							//������һ���۲�����
		}

		for(m=0;m<j;m++)
		{
			P[m*j+m]=weight[m];
		}

		if(j<=4)
			return -1;					//����۲����ݲ����ã����ܽ�����ջ�λ��
		else							//��С���˽���
		{
			matrix_transfer(j, 4, B, BT);
			matrix_multiply(4, j, j, j, BT, P, BTP);
			matrix_multiply(4, j, j, 4, BTP, B, BTPB);
			matrix_inverse(4, BTPB, BTPBinv);
			matrix_multiply(4, j, j, 1, BTP, L, BTPL);
			matrix_multiply(4, 4, 4, 1, BTPBinv, BTPL, delta);
		}
		k++;
	}while(sqrt(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2])>0.001 && k<10);		//ͬʱ��������������ѭ��

	//���о�������
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


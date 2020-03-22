/*********************************************************************
���㶨λģ�飺����α��۲�ֵ�����վ��λ�á��ٶȡ����ջ��Ӳ����

���ߣ�������
ʱ�䣺2017/5/18
*********************************************************************/
#include "stdafx.h"
#include"singlepoint.h"
#include"struct.h"
#include"matrix.h"
#include"space.h"
#include"solution.h"

/*********************************************************************
calsta

���ܣ����ݹ۲�ֵ�ṹ�壬�������������е��㶨λ�㷨��������ջ���λ�á��ٶȣ�
     �Ӳ����

������const struct RANGE *range���۲�ֵ�ṹ�壬�����˹۲�ʱ�䡢�۲��������
	  �Ͷ�Ӧ���ǵ�α��
      const struct GPSEPHEM *ephem���������������ڽ������ǵ�λ�á������Ӳ�
	  const struct IONUTC *ion�������������ģ�ͽṹ��
	  struct CALSTAPOS *stapos����Ž�������Ľ��ջ���λ�á��ٶȡ��Ӳ����
	  struct CALSATPOS satpos[]����Ž�������ĸ������ǵ�λ�á��ٶȡ��Ӳ����

����ֵ��0����ȷ���㣬-1��δ�ܽ���
*********************************************************************/
int calsta(const struct RANGE *range, const struct GPSEPHEM *ephem, const struct IONUTC *ion, struct CALSTAPOS *stapos, struct CALSATPOS satpos[])
{
	double delta[4];						//��С���˵Ĵ�������
	double staXYZ[4];						//����Ĳ�վ��XYZλ��
	double staBLH[4];						//�����վ��BLHλ��
	double L[NumOfChannel];					//L����
	memset(L, 0, sizeof(L));
	double B[4 * NumOfChannel];				//B����
	memset(B, 0, sizeof(B));
	int counter;							//��������¼���ϵ�������
	int i = 0;								//ѭ������i
	int j = 0;								//ѭ������j
	int k = 0;								//ѭ������k
	int m = 0;								//ѭ������m
	struct GPSTIME TSgps;					//�źŷ���ʱ�䣨GPSϵͳʱ��
	double satclkerr;						//ĳ���ǵ��������Ӳ�
	double transmitime;						//�źŴ���ʱ��
	double alpha;							//������ת�����Ƕ�
	double eTrans[9];						//������ת��������
	double satXYZ[3];						//�м��������ʱ������ǵ�λ��
	double ele;								//���Ǹ߶Ƚ�
	double azi;								//���Ƿ�λ��
	double ionDelay = 0;						//������ӳ٣��ף�
	double tropDelay = 0;						//�������ӳ٣��ף�
	double R;								//���ǵ����ջ��ľ���
	double BT[4 * NumOfChannel];				//B�����ת��
	double BTB[16];							//BT*B
	double BTBinv[16];						//BT*B��ת�þ���
	double BTL[4];							//BT*L
	double V[NumOfChannel];					//�в����
	double VT[NumOfChannel];				//�в�����ת��
	double BX[NumOfChannel];				//B*delta
	double VTV[1];							//VT*V
	double sigma;							//��λȨ�����
	double PDOP;							//λ�þ�������
	double DOP;								//��������

	staXYZ[0] = stapos->staXYZ[0];						//��վX�����ʼֵ����һ��Ԫ�����X����ֵ
	staXYZ[1] = stapos->staXYZ[1];						//��վY�����ʼֵ����һ��Ԫ�����Y����ֵ
	staXYZ[2] = stapos->staXYZ[2];						//��վZ�����ʼֵ����һ��Ԫ�����Z����ֵ
	staXYZ[3] = stapos->staClk;							//��վ���ջ��Ӳ��ֵ����һ��Ԫ������Ӳ�

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
			//���������źŷ���ʱ��
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
			}													//�������������ڣ���ȡ��һ���۲������ 
			if (satclkerr == -2)
			{
				i++; continue;
			}													//������û����������ȡ��һ���۲������ 
			if (satclkerr == -3)												//�����Ƿ�GPS���ǣ�������
			{
				i++; continue;
			}
			TSgps.SecOfWeek = TSgps.SecOfWeek - satclkerr;					//ȷ��ĳ�����źŵķ���ʱ��

			calsat(&ephem[range->Obs[i].PRN - 1], &TSgps, range->Obs[i].PRN, &satpos[i]);

			//��ʼ������ת����
			alpha = OMEGAedot*(range->ObsGPSTime.SecOfWeek - staXYZ[3] / C - TSgps.SecOfWeek);	//�źŴ���ʱ����ת���ĽǶ�
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
			//����������ת����

			if (sqrt(staXYZ[0] * staXYZ[0] + staXYZ[1] * staXYZ[1] + staXYZ[2] + staXYZ[2]) > 5000000
				&& sqrt(staXYZ[0] * staXYZ[0] + staXYZ[1] * staXYZ[1] + staXYZ[2] + staXYZ[2]) < 8000000)
			{
				calsatEA(staXYZ, satpos[i].satXYZ, &ele, &azi);
				satpos[i].eleAngle = ele;
				satpos[i].azimuth = azi;
				if ((ele * 180 / PI) < 10)
				{
					i++; continue;
				}				//���Ǹ߶Ƚǵ���10�㣬��ȡ��һ���۲������
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
			return -1;					//����۲����ݲ����ã����ܽ�����ջ�λ��
		else							//��С���˽���
		{
			matrix_transfer(j, 4, B, BT);
			matrix_multiply(4, j, j, 4, BT, B, BTB);
			matrix_inverse(4, BTB, BTBinv);
			matrix_multiply(4, j, j, 1, BT, L, BTL);
			matrix_multiply(4, 4, 4, 1, BTBinv, BTL, delta);
		}
		k++;
	} while (sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]) > 0.001 && k < 10);		//ͬʱ��������������ѭ��

																										//���о�������
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

���ܣ����������ʱ�䣬��Ӧ��������������������ǵ��������Ӳ�

������const struct GPSTIME *time������ʱ��
	 const struct GPSEPHEM *ephem�������ǵ�����
	 int PRN��ĳ���ǵ�PRN��

����ֵ��double���������Ӳ�;-1,��������;-2:û�и����ǵ�����

˵������������ǰҪ�ж������Ƿ�������
*********************************************************************/
double calsatclk(const struct GPSTIME *time, const struct GPSEPHEM *ephem, int PRN)
{
	if(PRN>31)
		return -3;					//�����Ƿ�GPS����
	if(ephem->PRN != PRN)			//������û������
		return -2;
	double tc;						//����ʱ����Ӳ�ο�ʱ��Ĳ�
	double deltaTr;					//�����ЧӦ����
	double F;

	int i = 0;
	double Mk;						//ƽ�����(rad)
	double E = 0;					//ƫ�����
	double Ek = 0;					//ƫ�����
	double N0;						//ƽ�����ٶ�
	double tk;						//����ʱ��������ο�ʱ��Ĳ�
	double N;	

	double satclkerr;				//�������Ӳ�

	N0= sqrt(GM_WGS84/(ephem->A*ephem->A*ephem->A));
	tk= (time->Week - ephem->Week)*604800 + 
		(time->SecOfWeek - ephem->Toe);
	if(tk> 7200)					//�ж������Ƿ����
		return -1;
	N= N0 + ephem->deltaN;			//��ƽ���˶����ٶȽ��и���(rad/s)
	Mk= ephem->M0 + N*tk;			//rad

	do
	{
		E = Ek;
		Ek= Mk + ephem->ecc*sin(E);
		i++;
	}while((fabs(Ek-E)>THRESHOLD)||i<10);		//ƫ�����(rad)

	tc =(time->Week - ephem->Week)*604800 + 
		(time->SecOfWeek - ephem->Toc);
	F= ((-2)*sqrt(1.0*GM_WGS84))/(C*C);
	deltaTr= F*ephem->ecc*sqrt(ephem->A)*sin(Ek);
	satclkerr= ephem->a0 + ephem->a1*tc + ephem->a2*tc*tc+deltaTr-ephem->Tgd;
	return satclkerr;
}

/*********************************************************************
calstavelocity

���ܣ����ݻ�ȡ�Ĺ۲����ݣ���һ������Ĺ۲����ǵĽ����������źŷ���ʱ�̣���
	����һ������Ĳ�վ�Ľ������������վ���ٶȡ�

������const struct RANGE *range���۲�����
	 const struct CALSATPOS satpos[]���۲����ǵĽ�����
	 struct CALSTAPOS *stapos����վ�Ľ�����

����ֵ��-1���������0����ȷ����
*********************************************************************/
int calstavelocity(const struct RANGE *range, const struct CALSATPOS satpos[], struct CALSTAPOS *stapos)
{
	int i=0;						//ѭ������i
	int j=0;						//ѭ������j
	int k=0;						//����
	double B[4*NumOfChannel];		//��ƾ���
	double L[NumOfChannel];			//L-C����
	double calXYZdot[4];			//�����Ĳ�������վ�ٶȺ�����
	double R;						//վ�Ǽ��ξ���
	double Lambda1;					//L1�źŵĲ���
	Lambda1= C/FreqL1;
	double BT[4*NumOfChannel];		//B��ת�þ���
	double BTP[4*NumOfChannel];		//BT*P
	double BTPB[16];				//BT*P*B
	double BTPBinv[16];				//BT*P*B�������
	double BTPL[4];					//BT*P*L
	double BX[NumOfChannel];		//B*calXYZdot
	double V[NumOfChannel];			//�в����
	double VT[NumOfChannel];		//�в�����ת��
	double VTPV[1];					//VT*P*V
	double VTP[NumOfChannel];		//VT*P
	double sigma;					//��λȨ�����
	double ele;						//ĳ���ǵĸ߶Ƚ�
	double azi;						//ĳ���ǵķ�λ��
	double P[NumOfChannel*NumOfChannel];
									//Ȩ��
	memset(P, 0, sizeof(P));
	double reP[NumOfChannel];		//��¼Ȩֵ���Ա��γɾ���

	while(i < range->NumofObs && j < range->NumofObs)
	{
		if(range->Obs[i].PRN == satpos[j].PRN)
		{
			if(satpos[j].eleAngle*180/PI < 10)
			{
				i++;					//��ȡ��һ���۲����ǵĹ۲�����
				j++;					//��ȡ��һ���۲����ǵĽ�����
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
			i++;						//������û�н���������ȡ��һ���۲����ǵĹ۲�����
			j++;continue;				//������û�н���������ȡ��һ���ǽ�����
		}
		i++;							//��ȡ��һ���۲����ǵĹ۲�����
		j++;							//��ȡ��һ���۲����ǵĽ�����
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
	
	//��������
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
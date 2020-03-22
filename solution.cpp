/*********************************************************************
����ģ�飺�������������������ǵ�λ�á��ٶȺ������Ӳ����
		 ���������������㡢������������

���ߣ�������
ʱ�䣺2017/5/15
*********************************************************************/
#include "stdafx.h"
#include"solution.h"
#include"struct.h"
#include"matrix.h"
#include"space.h"

/*********************************************************************
calsat

���ܣ��жϸ������Ƿ��������������Ƿ����
	���������������ǵ�λ�á��ٶȺ��Ӳ���Ϣ
	�������Ӧ���ǵĽ���ṹ����

������const struct GPSEPHEM *ephem�������Ƕ�Ӧ�������ṹ��
      const struct GPSTIME *time��ʱ�䣬�����ڸ�ʱ���µĽ��
	  int PRN�������ǹ����Ŀ����ǵĽ��
	  struct CALSATPOS satpos�����ǽ������ṹ��

����ֵ��������������Ӧ���ǵĽ������������Ǻ�
*********************************************************************/
int calsat(const struct GPSEPHEM * ephem, const struct GPSTIME * time, int PRN, struct CALSATPOS * satpos)
{
	if (PRN > 31)
		return -1;
	if (ephem->PRN != PRN || PRN == 0)				//û�и����ǵ�����
		return -1;
	else
	{
		//���������ڵ���ع�����ϵ�е�λ��
		double N0;						//ƽ�����ٶ�
		double tk;						//����ʱ��������ο�ʱ��Ĳ�
		double N;
		double Mk;						//ƽ�����(rad)
		double E = 0;					//ƫ�����
		double Ek = 0;					//ƫ�����
		int i = 0;
		double Vk;						//������(rad)
		double Phik;					//�����Ǿ�(rad)
		double Miuk;					//���������������Ǿ�(rad)
		double rk;						//������(m)
		double ik;						//�������������
		double xk;						//�����ڹ������ϵ��X����
		double yk;						//�����ڹ������ϵ��Y����
		double Omegak;					//������������㾭��

		double Ekdot;					//ƫ����ǵı仯�ٶ�
		double Phikdot;					//�����Ǿ�ı仯�ٶ�
		double Miukdot;					//�Ľ���������Ǿ�ı仯�ٶ�
		double rkdot;					//�����򾶵ı仯�ٶ�
		double ikdot;					//�����ǵı仯�ٶ�
		double Omegakdot;				//�����㾭�ȵı仯�ٶ�
		double Rdot[12];				//���������˶��ٶȵ�ת�ƾ���
		double xkdot;					//�����ڹ������ϵ������仯�ٶ�
		double ykdot;					//�����ڹ������ϵ������仯�ٶ�
		double XYdot[4];

		double tc;						//����ʱ����Ӳ�ο�ʱ��Ĳ�
		double deltaTr;					//�����ЧӦ����
		double F;
		double deltaTrdot;				//�����ЧӦ�����ĵ���

		N0 = sqrt(GM_WGS84 / (ephem->A*ephem->A*ephem->A));
		tk = (time->Week - ephem->Week) * 604800 +
			(time->SecOfWeek - ephem->Toe);
		if (tk > 7200)					//�ж������Ƿ����
			return 0;
		N = N0 + ephem->deltaN;			//��ƽ���˶����ٶȽ��и���(rad/s)
		Mk = ephem->M0 + N*tk;			//rad

		do
		{
			E = Ek;
			Ek = Mk + ephem->ecc*sin(E);
			i++;
		} while ((fabs(Ek - E) > THRESHOLD) || i < 10);		//ƫ�����(rad)

		Vk = atan2((sqrt(1 - ephem->ecc*ephem->ecc))*sin(Ek),
			cos(Ek) - ephem->ecc);					//������(rad)

		Phik = Vk + ephem->omega;					//�����Ǿ�(rad)
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
		//�������������ڵ���ع�����ϵ�е�λ��

		//���������ڵ���ع�����ϵ�е��ٶ�
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
		//�������������ڵ���ع�����ϵ�е��ٶ�

		//�������ǵ��Ӳ����
		tc = (time->Week - ephem->Week) * 604800 +
			(time->SecOfWeek - ephem->Toc);
		F = ((-2)*sqrt(1.0*GM_WGS84)) / (C*C);
		deltaTr = F*ephem->ecc*sqrt(ephem->A)*sin(Ek);
		satpos->satClk = ephem->a0 + ephem->a1*tc +
			ephem->a2*tc*tc + deltaTr - ephem->Tgd;

		deltaTrdot = F*ephem->ecc*sqrt(ephem->A)*cos(Ek)*Ekdot;
		satpos->satClkdot = ephem->a1 + 2 * ephem->a2*tc + deltaTrdot;
		//�����������ǵ��Ӳ����
	}
	satpos->PRN = PRN;
	return PRN;
}

/*********************************************************************
calsatSA

���ܣ����ݲ�վ�������ڵ���ع�����ϵ�е�XYZ���꣬�����ڲ�վվ������ϵ������
	�ĸ߶ȽǺͷ�λ�ǡ�

������const double staXYZ[3]����վ����
      const double satXYZ[3]����������
	  double *eleAngle��ָ��ʵ�θ߶Ƚǵĵ�ַ���߶Ƚǵ�λΪ���ȣ�ȡֵ��Χ-PI~PI
	  double *azimuth��ָ��ʵ�η�λ�ǵĵ�ַ����λ�ǵ�λΪ���ȣ�ȡֵ��ΧΪ0~2PI

����ֵ��0����ȷ����
*********************************************************************/
int calsatEA(const double staXYZ[3], const double satXYZ[3], double *eleAngle, double *azimuth)
{
	double staBLH[3];								//��վ�Ĵ�ؾ�γ�Ⱥ͸߳�
	double Trans[9];								//����ת�þ���
	double satsta[3];								//��վ�����ǵ�����
	double satxyz[3];								//������վ������ϵ������
	XYZToBLH(staXYZ, staBLH, a_WGS84, e2_WGS84);

	staBLH[0]=staBLH[0]*PI/180.0;					//��γ�ȵĵ�λתΪ����rad
	staBLH[1]=staBLH[1]*PI/180.0;					//�����ȵĵ�λתΪ����rad

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
		(sqrt(satxyz[0]*satxyz[0]+satxyz[1]*satxyz[1]+satxyz[2]*satxyz[2])));		//�߶Ƚǣ�rad
	*azimuth= atan2(satxyz[1],satxyz[0]);											//��λ�ǣ�rad
	if(*azimuth<0)
		*azimuth=(*azimuth)+2*PI;

	return 0;
}

/*********************************************************************
klobuchar

���ܣ����ݲ�վ��λ���Լ��۲�ʱ�̡���������ģ�ͽṹ��
	�������������

������const struct GPSTIME *time���۲�ʱ��
      double staXYZ[3]���û����ջ�������
	  double eleAngle���߶Ƚǣ��߶Ƚǵ�λΪ���ȣ�ȡֵ��Χ-PI~PI
	  double azimuth����λ�ǣ���λ�ǵ�λΪ���ȣ�ȡֵ��ΧΪ0~2PI
	  struct GPSEPHEM *ephem��ĳ�����ǵ�����

����ֵ��double �����ĸ���ֵ
*********************************************************************/
double klobuchar(const struct GPSTIME *time, const double staXYZ[3], double eleAngle, double azimuth, const struct IONUTC *ion)
{
	double staBLH[3];								//�û����ջ��ľ�γ�Ⱥ͸߳�
	double EA;										//�����վ�ʹ��̵����������ĵļн�
	double IPPLat;									//���̵��γ��
	double IPPLon;									//���̵�ľ���
	double IPPmagLat;								//�����ĵش�γ��
	double IPPmagLon;								//���̵�ĵشž���
	double IPPt;									//���̵�ĵ���ʱ��
	double Ai;										//������ӳٵķ�ֵ
	double Pi;										//������ӳٵ�����
	double Xi;										//������ӳٵ���λ
	double SF;										//������б����
	double IonDelay;								//������ӳ٣��룩
	XYZToBLH(staXYZ, staBLH, a_WGS84, e2_WGS84);	//��γ�ȵ�λΪ��

	staBLH[0]= staBLH[0]/180.0;						//��γ�ȵ�λת��Ϊ����
	staBLH[1]= staBLH[1]/180.0;						//�����ȵ�λת��Ϊ����
	eleAngle= eleAngle/PI;							//�߶Ƚǵ�λת��Ϊ����
	azimuth=  azimuth/PI;							//��λ�ǵ�λת��Ϊ����

	EA= (0.0137/(eleAngle+0.11))-0.022;					//����
	IPPLat= staBLH[0]+EA*cos(azimuth*PI);				//���㴩�̵��γ��
	if(IPPLat>0.416)
		IPPLat= 0.146;
	if(IPPLat<-0.416)
		IPPLat=-0.416;
	IPPLon= staBLH[1]+EA*sin(azimuth*PI)/cos(IPPLat*PI);	//���㴩�̵�ľ���
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

���ܣ����ݲ�վλ�á�����Ԫ�ء����Ǹ߶ȽǼ��������������ֵ

������const double staXYZ[3]����վ��λ��
	const double eleAngle�����Ǹ߶Ƚǣ���λΪ����

����ֵ��double �������ģ�͸���ֵ
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
	double staBLH[3];					//��վ��γ�ȡ����ȡ��߳�
	double TropDelay;					//�������ӳ٣��ף�
	eleAngle=eleAngle*180/PI;			//���߶Ƚǵ�λ�ɻ���ת��Ϊ��
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
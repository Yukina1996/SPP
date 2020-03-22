// RealTimeSPP.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <string.h>
#include "decode.h"
#include "matrix.h"
#include "rtcm.h"
#include "Serial.h"
#include "singlepoint.h"
#include "singlepointwithrtcm.h"
#include "sockets.h"
#include "solution.h"
#include "space.h"
#include "struct.h"
#include "time.h"

int main()
{
	FILE * fpos;						//���ջ�λ�ý���ļ�
	FILE * fvel;						//���ջ��ٶȽ���ļ�
	FILE * frtcm;						//���ջ���ֽ���ļ�

	int check;							//check:����1��ʾУ����ȷ������-1��ʾУ�����
	RANGE range;						//��ŵ�ǰ��õĹ۲�����
	GPSEPHEM ephem[32];					//���32�����Ǹ��Ե�����
	memset(ephem, 0, sizeof(ephem));
	IONUTC ionutc;						//��ŵ�������ģ��
	PSRPOS psrpos;						//��Ž��ջ�����Ķ�λ���
	GPSTIME time;						//��ǰGPSʱ��
	CALSATPOS satpos[NumOfChannel];
	memset(satpos, 0, sizeof(satpos));
	CALSTAPOS stapos;
	memset(&stapos, 0, sizeof(stapos));
	SATRTCM satrtcm[32];
	memset(satrtcm, 0, sizeof(satrtcm));
	RTCMDATA rtcmdata;
	double TNow;						//��ǰ�۲�ʱ��
	double dt;

	CSerial gps;
	SOCKET socket;

	int i;								//ѭ������
	int numofsat;						//����ֵ�а�����Ч���������Ĺ۲���
	unsigned char buff[4096];
	unsigned char databuffer[1024];
	unsigned char rtcmBuff[4096];

	int len;
	int validlen=0;
	int datalen;
	int rtcmlen;
	unsigned int cal_crc;
	int flag;
	char str[50];						//�������ð忨

	//���ò��򿪴���
	if (gps.Open(3, 9600) == FALSE)
	{
		printf("����δ��\n");
		return 0;
	}

	//���ð忨
	strcpy_s(str, "log ionutcb onchanged");
	len = strlen(str);
	str[len++] = 0x0D;  str[len++] = 0x0A;
	gps.SendData(str, len);

	strcpy_s(str, "log gpsephemb onchanged");
	len = strlen(str);
	str[len++] = 0x0D;  str[len++] = 0x0A;
	gps.SendData(str, len);

	strcpy_s(str, "log rangeb ontime 1");
	len = strlen(str);
	str[len++] = 0x0D;  str[len++] = 0x0A;
	gps.SendData(str, len);

	strcpy_s(str, "log psrposb ontime 1");
	len = strlen(str);
	str[len++] = 0x0D;  str[len++] = 0x0A;
	gps.SendData(str, len);

	if ((fpos = fopen("result1.txt", "wt")) == NULL)
	{
		printf("Cannot write this file");
		return 1;
	}
	fprintf(fpos, "����(s)\t\t����γ��(��)\t���ؾ���(��)\t���ش�ظ�(��)\t�����Ӳ�\tsigma\tDOP_pos\t\n");

	if ((fvel = fopen("result2.txt", "wt")) == NULL)
	{
		printf("Cannot write this file");
		return 1;
	}
	fprintf(fvel, "����(s)\t\tX�����ٶ�(m/s)\tY�����ٶ�(m/s)\tZ�����ٶ�(m/s)\t���ջ�����*C\tsigma\t\tDOP\n");

	if ((frtcm = fopen("result3.txt", "wt")) == NULL)
	{
		printf("Cannot write this file");
	}
	fprintf(frtcm, "����(s)\t\t����RTCM���ĵ�Z����ֵ\t����γ��(��)\t���ؾ���(��)\t���ش�ظ�(��)\tX���ף�\tY���ף�\tZ���ף�\tsigma\n");

	//�����磬��GNSS���Ļ�ò�ָ�����
	OpenDGPSSocket(socket);

	while (1)
	{
		Sleep(1000);
		len = datalen = 0;

		if (gps.ReadDataWaiting() > 0)
		{
			datalen = gps.ReadData(buff, 4096);
			flag = 0;
			for(; flag<datalen; )
			{
				flag = GetDataFromSerial(buff, databuffer, datalen, validlen, flag);

				cal_crc = crc32(databuffer, (validlen - 4));
				check = checkdata(cal_crc, databuffer, validlen);

				if (check == 1)
				{
					//��Buff���н���
					memset(&range, 0, sizeof(RANGE));
					if (getdata(databuffer, &range, ephem, &ionutc, &psrpos) == 2)
					{		
						//�������ȡ�Ĳ�ֵ��Ĵ���rtcmBuff����ĳ���Ϊrtcmlen
						rtcmlen = recv(socket, (char*)rtcmBuff, 4096, 0);
						//ʵʱ��ֶ�λ
						DecodeRTCM(rtcmBuff, rtcmlen, satrtcm, &rtcmdata);
						//�жϲ���ļ��Ƿ�����ʱЧ��			
						//��ǰ�۲�ʱ��תΪZ����ֵ
						TNow = fmod(range.ObsGPSTime.SecOfWeek, 3600);

						//����ļ���ʱ��͵�ǰ�۲�ʱ��Ĳ�ֵ
						dt = TNow - rtcmdata.zcount;
						if (dt < -1800)
						{
							dt += 3600;
						}
						else if (dt > 1800)
						{
							dt -= 3600;
						}

						if (abs(dt) < 5)
						{
							if (calstawithrtcm(&range, ephem, &ionutc, &stapos, satpos, satrtcm) == 0)
							{
								fprintf(fpos, "%6lf\t%10.6f\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t\n", range.ObsGPSTime.SecOfWeek, rtcmdata.zcount, stapos.staBLH[0], stapos.staBLH[1],
									stapos.staBLH[2], stapos.staXYZ[0], stapos.staXYZ[1], stapos.staXYZ[2], stapos.sigmaP);			//�ļ����

								printf("%6lf\t%10.6f\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\n", range.ObsGPSTime.SecOfWeek, rtcmdata.zcount, stapos.staBLH[0], stapos.staBLH[1],
									stapos.staBLH[2], stapos.staClk);			//����̨���
							}
							else
							{
								//printf("this epoch error\n");
							}
						}						

						////SPP���㶨λ
						/*if (calsta(&range, ephem, &ionutc, &stapos, satpos) == 0)
						{
							fprintf(fpos, "%6lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\n", range.ObsGPSTime.SecOfWeek, stapos.staBLH[0], stapos.staBLH[1],
								stapos.staBLH[2], stapos.staClk, stapos.sigmaP, stapos.DOP_pos);

							printf("%6lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\n", range.ObsGPSTime.SecOfWeek, stapos.staBLH[0], stapos.staBLH[1],
								stapos.staBLH[2], stapos.staClk, stapos.sigmaP, stapos.DOP_pos);
						}

						else
							printf("this epoch error in single point positioning!\n");

						if ((calstavelocity(&range, satpos, &stapos)) == 0)
							fprintf(fvel, "%6lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\n", range.ObsGPSTime.SecOfWeek, stapos.staXYZdot[0], stapos.staXYZdot[1],
								stapos.staXYZdot[2], stapos.staClkdot, stapos.sigmaV, stapos.DOP_v);
						else
							printf("%lf\n", range.ObsGPSTime.SecOfWeek);*/

						memset(satpos, 0, sizeof(satpos));
						memset(satrtcm, 0, sizeof(satrtcm));
					}

				}		

			}
			memset(buff, 0, 4096);//ÿ��ѭ�����һ��buff
		}
	}

	gps.Close();
	return 0;
}



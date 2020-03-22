// RealTimeSPP.cpp : 定义控制台应用程序的入口点。
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
	FILE * fpos;						//接收机位置结果文件
	FILE * fvel;						//接收机速度结果文件
	FILE * frtcm;						//接收机差分结果文件

	int check;							//check:等于1表示校验正确，等于-1表示校验错误
	RANGE range;						//存放当前获得的观测数据
	GPSEPHEM ephem[32];					//存放32颗卫星各自的星历
	memset(ephem, 0, sizeof(ephem));
	IONUTC ionutc;						//存放电离层改正模型
	PSRPOS psrpos;						//存放接收机解算的定位结果
	GPSTIME time;						//当前GPS时间
	CALSATPOS satpos[NumOfChannel];
	memset(satpos, 0, sizeof(satpos));
	CALSTAPOS stapos;
	memset(&stapos, 0, sizeof(stapos));
	SATRTCM satrtcm[32];
	memset(satrtcm, 0, sizeof(satrtcm));
	RTCMDATA rtcmdata;
	double TNow;						//当前观测时间
	double dt;

	CSerial gps;
	SOCKET socket;

	int i;								//循环变量
	int numofsat;						//测量值中包含有效卫星星历的观测数
	unsigned char buff[4096];
	unsigned char databuffer[1024];
	unsigned char rtcmBuff[4096];

	int len;
	int validlen=0;
	int datalen;
	int rtcmlen;
	unsigned int cal_crc;
	int flag;
	char str[50];						//用来配置板卡

	//配置并打开串口
	if (gps.Open(3, 9600) == FALSE)
	{
		printf("串口未打开\n");
		return 0;
	}

	//配置板卡
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
	fprintf(fpos, "周秒(s)\t\t当地纬度(度)\t当地经度(度)\t当地大地高(米)\t卫星钟差\tsigma\tDOP_pos\t\n");

	if ((fvel = fopen("result2.txt", "wt")) == NULL)
	{
		printf("Cannot write this file");
		return 1;
	}
	fprintf(fvel, "周秒(s)\t\tX方向速度(m/s)\tY方向速度(m/s)\tZ方向速度(m/s)\t接收机钟速*C\tsigma\t\tDOP\n");

	if ((frtcm = fopen("result3.txt", "wt")) == NULL)
	{
		printf("Cannot write this file");
	}
	fprintf(frtcm, "周秒(s)\t\t最新RTCM电文的Z计数值\t当地纬度(度)\t当地经度(度)\t当地大地高(米)\tX（米）\tY（米）\tZ（米）\tsigma\n");

	//打开网络，从GNSS中心获得差分改正数
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
					//对Buff进行解码
					memset(&range, 0, sizeof(RANGE));
					if (getdata(databuffer, &range, ephem, &ionutc, &psrpos) == 2)
					{		
						//从网络获取的差分电文存在rtcmBuff里，电文长度为rtcmlen
						rtcmlen = recv(socket, (char*)rtcmBuff, 4096, 0);
						//实时差分定位
						DecodeRTCM(rtcmBuff, rtcmlen, satrtcm, &rtcmdata);
						//判断差分文件是否满足时效性			
						//当前观测时间转为Z计数值
						TNow = fmod(range.ObsGPSTime.SecOfWeek, 3600);

						//差分文件的时间和当前观测时间的差值
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
									stapos.staBLH[2], stapos.staXYZ[0], stapos.staXYZ[1], stapos.staXYZ[2], stapos.sigmaP);			//文件输出

								printf("%6lf\t%10.6f\t%10.8lf\t%10.8lf\t%10.8lf\t%10.8lf\n", range.ObsGPSTime.SecOfWeek, rtcmdata.zcount, stapos.staBLH[0], stapos.staBLH[1],
									stapos.staBLH[2], stapos.staClk);			//控制台输出
							}
							else
							{
								//printf("this epoch error\n");
							}
						}						

						////SPP单点定位
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
			memset(buff, 0, 4096);//每次循环清空一次buff
		}
	}

	gps.Close();
	return 0;
}



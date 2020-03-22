/*********************************************************************
从获得的数据中提取需要的数据项，过程包括CRC校验和各卫星星历数据提取、电离层
改正数据提取、各卫星观测数据提取、定位数据提取


作者：王雅仪
时间：2017/5/8
*********************************************************************/
#include "stdafx.h"

#include "decode.h"
#include "struct.h"
#include "..\..\ConsoleApplication1\ConsoleApplication1\decode.h"

#define OEMSYNC1   0xAA       
#define OEMSYNC2   0x44     
#define OEMSYNC3   0x12   
#define OEM4HLEN   28
/*********************************************************************
D8,F4,UL4,,L4,I4,S2

功能：解码，将二进制转为double/float/unsigned long/long/unsigned int/
		unsigned short

参数：const unsigned char *p:指向要转化的二进制数据

返回值：该二进制数据解码为double/float/unsigned long/long/unsigned int/
		unsigned short的值

*********************************************************************/
double D8(const unsigned char *p)
{
	double r;
	memcpy(&r,p,8);
	return r;
}

float F4(const unsigned char *p)
{
	float r;
	memcpy(&r,p,4);
	return r;
}

unsigned long UL4(const unsigned char *p)
{
	unsigned long r;
	memcpy(&r,p,4);
	return r;
}

long L4(const unsigned char *p)
{
	long r;
	memcpy(&r,p,4);
	return r;
}

unsigned int UI4(const unsigned char *p)
{
	unsigned int r;
	memcpy(&r,p,4);
	return r;
}

unsigned short S2(const unsigned char *p)
{
	unsigned short r;
	memcpy(&r,p,2);
	return r;
}

/*********************************************************************
GetDataFromSerial

功能：从串口读取OEMStar板卡发来的数据

参数：unsigned char * buff: 用来存储找到同步字的数据
int &l length: 接收到的数据的长度
int i:记录在buff中同步字开始的位置
int validlen:一段有效数据的长度

算法：寻找同步字

*********************************************************************/
int GetDataFromSerial(unsigned char * buff, unsigned char * databuffer, const int & length, int & validlen, int flag)
{
	int i,j;
	i = flag;
	int messagelen = 0;

	for (; i < length; i++)
	{
		if (buff[i] == OEMSYNC1 && buff[i + 1] == OEMSYNC2 && buff[i + 2] == OEMSYNC3)
		{
			break;
		}
	}

	if (i + OEM4HLEN >= length)         
		return length;
	for (j = 0; j < OEM4HLEN; j++)   
		databuffer[j] = buff[i + j];
	messagelen = (int)(buff[i + 9] << 8 | buff[i + 8]) + OEM4HLEN;
	if ((messagelen + 4 + i) > length) 	      
		return length;
	for (j = OEM4HLEN; j < messagelen + 4; j++) 
		databuffer[j] = buff[i + j];
	
	validlen = messagelen + 4;		//header+body+crc

	return i + validlen;
}

/*********************************************************************
crc32

功能：用获得的数据的头和体来计算CRC

参数：const unsigned char *buff：存放读取的数据
      int len：数据的头和体的总长度

返回值：crc：计算得到的CRC码

*********************************************************************/
unsigned int crc32(const unsigned char *buff, int len)
{
    int i,j;
    unsigned int crc=0;

    for (i=0;i<len;i++)
    {
        crc^=buff[i];
        for (j=0;j<8;j++) 
        {
            if (crc&1) crc=(crc>>1)^POLYCRC32; 
            else crc>>=1;
        }
    }
    return crc;
}

/*********************************************************************
checkdata

功能：检查计算得到的CRC码和真实获得的CRC码是否一致
      如果一致，说明获得的数据没问题，可以进行接下来的解码和定位计算

参数：const unsigned int cal_crc：计算得到的CRC码
      const unsigned char *buff：存放读取的数据
      int len：数据的头和体的总长度

*********************************************************************/
int checkdata(const unsigned int cal_crc, const unsigned char *buff, int len)
{
	unsigned char crc[4];
	unsigned int real_crc;
	real_crc= UI4(&buff[len-4]);

	if(real_crc==cal_crc)
		return 1;
	else
	{
		printf("串口接收数据，crc校验出错\n");
	    return -1;
	}
}


/*********************************************************************
getdata

功能：从一段语句中提取所需的数据，存放在相应的结构体中

参数：const unsigned char *buff:存放读取的数据
      int bufflen: Buff有效数据的长度
	  struct RANGE *：指向RANGE结构体的指针
	  struct GPSEPHEM *：指向GPSEPHEM结构体的指针
	  struct IONUTC *：指向IONUTC结构体的指针
	  struct PSRPOS *: 指向接收机输出的定位结果
*********************************************************************/
int getdata(const unsigned char *buff, struct RANGE * range, struct GPSEPHEM ephem[], struct IONUTC *ionutc, 
	struct PSRPOS *psrpos)
{
	//从头中读取Message ID
	unsigned short messageID;
	unsigned int headerlen;
	unsigned short week;
	long ms;

	messageID= S2(&buff[4]);
	headerlen= buff[3];
	week= S2(&buff[14]);
	ms= UL4(&buff[16]);

	switch(messageID)
	{
	case 43:
		{
			//读取观测数据
			range->ObsGPSTime.Week= week;
			range->ObsGPSTime.SecOfWeek= ms*0.001;			//单位由ms转换为s

			range->NumofObs= buff[headerlen];

			for(int i=0; i<range->NumofObs; i++)
			{
				range->Obs[i].PRN= buff[headerlen+5+(i*44)]<<8|buff[headerlen+4+(i*44)];
				range->Obs[i].Psr= D8(&buff[headerlen+8+(i*44)]);
				range->Obs[i].PsrStd= F4(&buff[headerlen+16+(i*44)]);
				range->Obs[i].Adr= D8(&buff[headerlen+20+(i*44)]);
				range->Obs[i].AdrStd= F4(&buff[headerlen+28+(i*44)]);
				range->Obs[i].Dopp= F4(&buff[headerlen+32+(i*44)]);
				range->Obs[i].CND= F4(&buff[headerlen+36+(i*44)]);
			}		
			return 2;  //返回2，说明正确返回该历元的观测数据
		}
	case 7:
	{
		//读取星历数据
		int i;
		i = buff[headerlen];
		if (i >= 1 && i <= 32)
		{
			i--;
			ephem[i].PRN = UL4(&buff[headerlen]);
			ephem[i].Tow = D8(&buff[headerlen + 4]);
			ephem[i].Health = UL4(&buff[headerlen + 12]);
			ephem[i].IODE = UL4(&buff[headerlen + 16]);
			ephem[i].Week = UL4(&buff[headerlen + 24]);
			ephem[i].zWeek = UL4(&buff[headerlen + 28]);
			ephem[i].Toe = D8(&buff[headerlen + 32]);
			ephem[i].A = D8(&buff[headerlen + 40]);
			ephem[i].deltaN = D8(&buff[headerlen + 48]);
			ephem[i].M0 = D8(&buff[headerlen + 56]);
			ephem[i].ecc = D8(&buff[headerlen + 64]);
			ephem[i].omega = D8(&buff[headerlen + 72]);
			ephem[i].cuc = D8(&buff[headerlen + 80]);
			ephem[i].cus = D8(&buff[headerlen + 88]);
			ephem[i].crc = D8(&buff[headerlen + 96]);
			ephem[i].crs = D8(&buff[headerlen + 104]);
			ephem[i].cic = D8(&buff[headerlen + 112]);
			ephem[i].cis = D8(&buff[headerlen + 120]);
			ephem[i].I0 = D8(&buff[headerlen + 128]);
			ephem[i].RofI = D8(&buff[headerlen + 136]);
			ephem[i].omega0 = D8(&buff[headerlen + 144]);
			ephem[i].RofOmega0 = D8(&buff[headerlen + 152]);
			ephem[i].Toc = D8(&buff[headerlen + 164]);
			ephem[i].Tgd = D8(&buff[headerlen + 172]);
			ephem[i].a0 = D8(&buff[headerlen + 180]);
			ephem[i].a1 = D8(&buff[headerlen + 188]);
			ephem[i].a2 = D8(&buff[headerlen + 196]);
			ephem[i].N = D8(&buff[headerlen + 208]);
			return 3;			//返回3，说明正确返回星历数据
		}

		else
			return -3;
	}
	case 8:
		{
			//读取电离层改正数据
			ionutc->a0= D8(&buff[headerlen]);
			ionutc->a1= D8(&buff[headerlen+8]);
			ionutc->a2= D8(&buff[headerlen+16]);
			ionutc->a3= D8(&buff[headerlen+24]);
			ionutc->b0= D8(&buff[headerlen+32]);
			ionutc->b1= D8(&buff[headerlen+40]);
			ionutc->b2= D8(&buff[headerlen+48]);
			ionutc->b3= D8(&buff[headerlen+56]);
			ionutc->UtcWn= UL4(&buff[headerlen+64]);
			ionutc->Tot= UL4(&buff[headerlen+68]);
			ionutc->A0 = D8(&buff[headerlen+72]);
			ionutc->A1 = D8(&buff[headerlen+80]);
			ionutc->WnLsf= UL4(&buff[headerlen+88]);
			ionutc->Dn= UL4(&buff[headerlen+92]);
			ionutc->deltatLs= L4(&buff[headerlen+96]);
			ionutc->deltatLsf= L4(&buff[headerlen+100]);
			ionutc->deltatUtc= UL4(&buff[headerlen+108]);

			return 4;		//返回4，说明正确返回电离层改正模型
		}
	case 47:
		{
			//读取定位结果数据
			psrpos->ObsGPSTime.Week= week;
			psrpos->ObsGPSTime.SecOfWeek= ms*0.001;

			psrpos->NovLat= D8(&buff[headerlen+8]);
			psrpos->NovLon= D8(&buff[headerlen+16]);
			psrpos->NovHgt= D8(&buff[headerlen+24]);
			psrpos->NovUndulation= F4(&buff[headerlen+32]);

			return 5;		//返回5，说明正确返回定位结果
		}
	default:
		return -1;//不是可以提取的数据
	}

	return 0;
}


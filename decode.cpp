/*********************************************************************
�ӻ�õ���������ȡ��Ҫ����������̰���CRCУ��͸���������������ȡ�������
����������ȡ�������ǹ۲�������ȡ����λ������ȡ


���ߣ�������
ʱ�䣺2017/5/8
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

���ܣ����룬��������תΪdouble/float/unsigned long/long/unsigned int/
		unsigned short

������const unsigned char *p:ָ��Ҫת���Ķ���������

����ֵ���ö��������ݽ���Ϊdouble/float/unsigned long/long/unsigned int/
		unsigned short��ֵ

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

���ܣ��Ӵ��ڶ�ȡOEMStar�忨����������

������unsigned char * buff: �����洢�ҵ�ͬ���ֵ�����
int &l length: ���յ������ݵĳ���
int i:��¼��buff��ͬ���ֿ�ʼ��λ��
int validlen:һ����Ч���ݵĳ���

�㷨��Ѱ��ͬ����

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

���ܣ��û�õ����ݵ�ͷ����������CRC

������const unsigned char *buff����Ŷ�ȡ������
      int len�����ݵ�ͷ������ܳ���

����ֵ��crc������õ���CRC��

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

���ܣ�������õ���CRC�����ʵ��õ�CRC���Ƿ�һ��
      ���һ�£�˵����õ�����û���⣬���Խ��н������Ľ���Ͷ�λ����

������const unsigned int cal_crc������õ���CRC��
      const unsigned char *buff����Ŷ�ȡ������
      int len�����ݵ�ͷ������ܳ���

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
		printf("���ڽ������ݣ�crcУ�����\n");
	    return -1;
	}
}


/*********************************************************************
getdata

���ܣ���һ���������ȡ��������ݣ��������Ӧ�Ľṹ����

������const unsigned char *buff:��Ŷ�ȡ������
      int bufflen: Buff��Ч���ݵĳ���
	  struct RANGE *��ָ��RANGE�ṹ���ָ��
	  struct GPSEPHEM *��ָ��GPSEPHEM�ṹ���ָ��
	  struct IONUTC *��ָ��IONUTC�ṹ���ָ��
	  struct PSRPOS *: ָ����ջ�����Ķ�λ���
*********************************************************************/
int getdata(const unsigned char *buff, struct RANGE * range, struct GPSEPHEM ephem[], struct IONUTC *ionutc, 
	struct PSRPOS *psrpos)
{
	//��ͷ�ж�ȡMessage ID
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
			//��ȡ�۲�����
			range->ObsGPSTime.Week= week;
			range->ObsGPSTime.SecOfWeek= ms*0.001;			//��λ��msת��Ϊs

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
			return 2;  //����2��˵����ȷ���ظ���Ԫ�Ĺ۲�����
		}
	case 7:
	{
		//��ȡ��������
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
			return 3;			//����3��˵����ȷ������������
		}

		else
			return -3;
	}
	case 8:
		{
			//��ȡ������������
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

			return 4;		//����4��˵����ȷ���ص�������ģ��
		}
	case 47:
		{
			//��ȡ��λ�������
			psrpos->ObsGPSTime.Week= week;
			psrpos->ObsGPSTime.SecOfWeek= ms*0.001;

			psrpos->NovLat= D8(&buff[headerlen+8]);
			psrpos->NovLon= D8(&buff[headerlen+16]);
			psrpos->NovHgt= D8(&buff[headerlen+24]);
			psrpos->NovUndulation= F4(&buff[headerlen+32]);

			return 5;		//����5��˵����ȷ���ض�λ���
		}
	default:
		return -1;//���ǿ�����ȡ������
	}

	return 0;
}


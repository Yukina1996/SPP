/*********************************************************************
从RTCM的文件中读取差分改正数，单独存放每颗卫星的差分改正数

作者：王雅仪
时间：2017/6/7
*********************************************************************/
#include "stdafx.h"
#include "rtcm.h"
#include "struct.h"

BYTE rot[5];						//5*6位有效信息
BYTE unite[4];						//24位有效信息+6位奇偶校验信息

static BYTE ParityD29=1;
static BYTE ParityD30=1;
/*********************************************************************
RTCMroll

功能：将8bit的unsigned char数据进行位运算，得到字节扫描和滚动后的6bit数据，
	但6bit的数据仍然存在unsigned char数据里

参数：char kch；一个8bit的字节

返回值：返回低6位并且滚动后的数据

*********************************************************************/
BYTE RTCMRoll(BYTE kch)
{
	char rtcm[7];
	rtcm[0]=kch>>5&0x01;
	rtcm[1]=kch>>3&0x02;
	rtcm[2]=kch>>1&0x04;
	rtcm[3]=kch<<1&0x08;
	rtcm[4]=kch<<3&0x10;
	rtcm[5]=kch<<5&0x20;
	rtcm[6]=rtcm[0]|rtcm[1]|rtcm[2]|rtcm[3]|rtcm[4]|rtcm[5];
	return rtcm[6];
}

/*********************************************************************
combine_to_word

功能：将5个6bit字节联合成3个8bit字节和一个6bit字节

*********************************************************************/
void combine_to_word(BYTE rot[], BYTE words[])
{      
	words[0]=(rot[0]<<2)|(rot[1]>>4);
	words[1]=(rot[1]<<4)|(rot[2]>>2);
	words[2]=(rot[2]<<6)|rot[3];
	words[3]=rot[4];
}

/*********************************************************************
byte_jump_page

功能：不能找到同步字时，字节跳页。

*********************************************************************/
void byte_jump_page(BYTE word[])
{
	word[0]=word[0]<<6|word[1]>>2;
	word[1]=word[1]<<6|word[2]>>2;
	word[2]=word[2]<<6|(word[3]&0x3F);
}
/*********************************************************************
RTCMParity

功能：进行奇偶校验

参数：const BYTE unite[]：输入待检验的24位+6位信息
	BYTE rem：上一个字的后两位

*********************************************************************/
bool RTCMParity(BYTE word[])
{
	BYTE d[6]={0};
	BYTE unite=0;

	d[0]=ParityD29^(word[0]>>7&0x01)^(word[0]>>6&0x01)^(word[0]>>5&0x01)^(word[0]>>3&0x01)^(word[0]>>2&0x01)^
		(word[1]>>6&0x01)^(word[1]>>5&0x01)^(word[1]>>4&0x01)^(word[1]>>3&0x01)^(word[1]>>2&0x01)^
		(word[2]>>7&0x01)^(word[2]>>6&0x01)^(word[2]>>4&0x01)^(word[2]>>1&0x01);

	d[1]=ParityD30^(word[0]>>6&0x01)^(word[0]>>5&0x01)^(word[0]>>4&0x01)^(word[0]>>2&0x01)^(word[0]>>1&0x01)^
		(word[1]>>5&0x01)^(word[1]>>4&0x01)^(word[1]>>3&0x01)^(word[1]>>2&0x01)^(word[1]>>1&0x01)^
		(word[2]>>6&0x01)^(word[2]>>5&0x01)^(word[2]>>3&0x01)^(word[2]&0x01);

	d[2]=ParityD29^(word[0]>>7&0x01)^(word[0]>>5&0x01)^(word[0]>>4&0x01)^(word[0]>>3&0x01)^(word[0]>>1&0x01)^(word[0]&0x01)^
		(word[1]>>4&0x01)^(word[1]>>3&0x01)^(word[1]>>2&0x01)^(word[1]>>1&0x01)^(word[1]&0x01)^
		(word[2]>>5&0x01)^(word[2]>>4&0x01)^(word[2]>>2&0x01);

	d[3]=ParityD30^(word[0]>>6&0x01)^(word[0]>>4&0x01)^(word[0]>>3&0x01)^(word[0]>>2&0x01)^(word[0]&0x01)^
		(word[1]>>7&0x01)^(word[1]>>3&0x01)^(word[1]>>2&0x01)^(word[1]>>1&0x01)^(word[1]&0x01)^
		(word[2]>>7&0x01)^(word[2]>>4&0x01)^(word[2]>>3&0x01)^(word[2]>>1&0x01);

	d[4]=ParityD30^(word[0]>>7&0x01)^(word[0]>>5&0x01)^(word[0]>>3&0x01)^(word[0]>>2&0x01)^(word[0]>>1&0x01)^
		(word[1]>>7&0x01)^(word[1]>>6&0x01)^(word[1]>>2&0x01)^(word[1]>>1&0x01)^(word[1]&0x01)^
		(word[2]>>7&0x01)^(word[2]>>6&0x01)^(word[2]>>3&0x01)^(word[2]>>2&0x01)^(word[2]&0x01);

	d[5]=ParityD29^(word[0]>>5&0x01)^(word[0]>>3&0x01)^(word[0]>>2&0x01)^(word[0]&0x01)^
		(word[1]>>7&0x01)^(word[1]>>6&0x01)^(word[1]>>5&0x01)^(word[1]>>3&0x01)^(word[1]>>1&0x01)^
		(word[2]>>5&0x01)^(word[2]>>2&0x01)^(word[2]>>1&0x01)^(word[2]&0x01);

	unite=d[5]|(d[4]<<1)|(d[3]<<2)|(d[2]<<3)|(d[1]<<4)|(d[0]<<5);

	if(unite==word[3])  
		return true;
	else 
		return false;	

}
/*********************************************************************
findpreamble

功能：找到引导字（01100110）的位置，从引导字以后开始解码

*********************************************************************/
bool findpreamble(FILE* fin,BYTE raw[],BYTE w[])
{
	bool complement=0;		 //补码

	do{
		if(ParityD30==1) 
		{
			for(int i=0;i<3;i++) w[i]=~w[i];
			complement=1;                                
		}

		if(w[0]==102)  return true;		//找到同步字

		if(complement==1) 
		{
			for(int i=0;i<3;i++) w[i]=~w[i];
			complement=0;                                
		}

		ParityD30=w[0]>>2&0x01;
		ParityD29=w[0]>>3&0x01;

		byte_jump_page(w);				//字节跳页
		for(int i=0;i<4;i++)    
			raw[i]=raw[i+1];
		fread(&raw[4], sizeof(BYTE), 1, fin);

		w[3]=RTCMRoll(raw[4]);

	}while(!feof(fin));
	return true;
}

/*********************************************************************
重载：findpreamble（从buffer中解码rtcm电文）

参数：buff[]:存放rtcm电文的缓冲区
		j:已读到的缓冲区的位置
		n:rtcm电文的长度

功能：找到引导字（01100110）的位置，从引导字以后开始解码

*********************************************************************/
bool findpreamble(BYTE buff[], int & j, int n, BYTE raw[], BYTE w[])
{
	bool complement = 0;		 //补码

	do {
		if (ParityD30 == 1)
		{
			for (int i = 0; i < 3; i++) w[i] = ~w[i];
			complement = 1;
		}

		if (w[0] == 102)
		{
			for (int i = 0; i < 3; i++)
				w[i] = ~w[i];
			return true;		//找到同步字
		}

		if (complement == 1)
		{
			for (int i = 0; i < 3; i++) w[i] = ~w[i];
			complement = 0;
		}

		ParityD30 = w[0] >> 2 & 0x01;
		ParityD29 = w[0] >> 3 & 0x01;

		byte_jump_page(w);				//字节跳页
		for (int i = 0; i < 4; i++)
			raw[i] = raw[i + 1];
		raw[4] = buff[j]; j++;
		if (j >= n)
		{
			return false;
		}
		w[3] = RTCMRoll(raw[4]);

	} while (j < n);
	return true;
}

/*********************************************************************
DecodeRTCMHead

功能：解码这段RTCM电文的文件头（两个字）

*********************************************************************/
bool DecodeRTCMHead(FILE *fin, BYTE raw[],BYTE w[],struct RTCMDATA *rtcmdata)
{
	BYTE rot[5]; 
	unsigned short zcount=0;

	if(RTCMParity(w)==0)
	{
		ParityD30=w[3]&0x01;
		ParityD29=w[3]>>1&0x01;
		rtcmdata->flag=1;
		return false;  //校验错误
	}

	rtcmdata->type=w[1]>>2;
	rtcmdata->id=((w[1]<<8)&0x0300)|w[2];

	ParityD29=w[3]>>1&0x01;  
	ParityD30=w[3]&0x01;

	for(int i=0;i<5;)								//读5个字节
	{
		fread(&raw[i], sizeof(BYTE), 1, fin);
		if((raw[i]>>7)==0&&((raw[i]>>6)&0x01)==1)  
		{
			rot[i]=RTCMRoll(raw[i]);
			i++;        
		}
	}

	combine_to_word(rot,w);
	if(ParityD30==1)   for(int i=0;i<3;i++)  w[i]=~w[i];
	if(RTCMParity(w)==0)
	{
		ParityD30=w[3]&0x01;
		ParityD29=w[3]>>1&0x01;
		rtcmdata->flag=1;
		return false;	    //校验错误
	}

	zcount=w[0]<<5|((w[1]>>3)&0x1F);

	rtcmdata->zcount=zcount*0.6;
	rtcmdata->seqno=w[1]&0x07;
	rtcmdata->length=(w[2]>>3)&0x1F;
	rtcmdata->status=w[2]&0x07;
	
	ParityD29=w[3]>>1&0x01;  
	ParityD30=w[3]&0x01;

	return true;
}

/*********************************************************************
DecodeRTCMHead

功能：解码这段RTCM电文的文件头（两个字）

*********************************************************************/
bool DecodeRTCMHead(BYTE buff[], int & j, int n, BYTE raw[], BYTE w[], struct RTCMDATA * rtcmdata)
{
	BYTE rot[5];
	unsigned short zcount = 0;

	if (ParityD30 == 1)  for (int i = 0; i<3; i++)  w[i] = ~w[i];

	if (RTCMParity(w) == 0)
	{
		ParityD30 = w[3] & 0x01;
		ParityD29 = w[3] >> 1 & 0x01;
		rtcmdata->flag = 1;
		return false;  //校验错误
	}

	rtcmdata->type = w[1] >> 2;
	rtcmdata->id = ((w[1] << 8) & 0x0300) | w[2];

	ParityD29 = w[3] >> 1 & 0x01;
	ParityD30 = w[3] & 0x01;

	for (int i = 0; i < 5;)								//读5个字节
	{
		raw[i] = buff[j]; j++;
		if (j >= n)
			return false;
		if ((raw[i] >> 7) == 0 && ((raw[i] >> 6) & 0x01) == 1)
		{
			rot[i] = RTCMRoll(raw[i]);
			i++;
		}
	}

	combine_to_word(rot, w);
	if (ParityD30 == 1)   for (int i = 0; i < 3; i++)  w[i] = ~w[i];
	if (RTCMParity(w) == 0)
	{
		ParityD30 = w[3] & 0x01;
		ParityD29 = w[3] >> 1 & 0x01;
		rtcmdata->flag = 1;
		return false;	    //校验错误
	}

	zcount = w[0] << 5 | ((w[1] >> 3) & 0x1F);

	rtcmdata->zcount = zcount*0.6;
	rtcmdata->seqno = w[1] & 0x07;
	//文件中，长度为5位，健康状况为3位
	//如果实时接收，长度为6位，健康状况为2位
	//rtcmdata->length = (w[2] >> 3) & 0x1F;
	//rtcmdata->status = w[2] & 0x07;
	rtcmdata->length = (w[2] >> 2) & 0x3F;
	rtcmdata->status = w[2] & 0x03;

	ParityD29 = w[3] >> 1 & 0x01;
	ParityD30 = w[3] & 0x01;

	return true;
}
/*********************************************************************
DecodeRTCMBody

功能：解码这段RTCM电文的正文部分（N个字）

*********************************************************************/
void DecodeRTCMBody(FILE* fin, struct RTCMDATA *rtcmdata,int len, struct SATRTCM satrtcm[])
{
	int i, n,sum=0;
	BYTE raw[250], rot[250], words[200];
	int satnum=(int)(0.6*len);
	short d2=0;
	char d1=0;

	for(i=0;i<len;i++)			//读N*5个字节
	{
		for(int j=0;j<5;)
		{
			fread(&raw[i*5+j], sizeof(BYTE), 1, fin);

			if((raw[i*5+j]>>7)==0&&((raw[i*5+j]>>6)&0x01)==1)   
			{
				rot[i*5+j]=RTCMRoll(raw[i*5+j]);
				j++;        
			}
		}

		combine_to_word(&rot[i*5],&words[i*4]);
		if(ParityD30==1)  for(int j=0;j<3;j++) words[i*4+j]=~words[i*4+j];

		if(RTCMParity(&words[i*4])==0)		//奇偶校验失败
			return;
		ParityD30=words[i*4+3]&0x01; 
		ParityD29=(words[i*4+3]>>1)&0x01;
	}

	for(i=2;i<4*len;i++)  
	{
		if(fmod(i+1,4.0)==0) {
			sum+=1;
			continue;
		}
		words[i-sum]=words[i];
	}

	sum=0;
	for(i=0;i<satnum;i++)
	{
		rtcmdata->rtcmbody[i].scale=(words[sum]>>7)&0x01;
		rtcmdata->rtcmbody[i].UDRE=(words[sum]>>5)&0x03;
		rtcmdata->rtcmbody[i].SatID=words[sum]&0x1F;

		if (rtcmdata->rtcmbody[i].SatID == 0)
		{
			rtcmdata->rtcmbody[i].SatID == 32;//0表示的是32号卫星
		}

		d2=words[sum+1]<<8|words[sum+2];		
		rtcmdata->rtcmbody[i].PRC0=d2*(0.02+0.3*rtcmdata->rtcmbody[i].scale);		//伪距改正值
		d1=words[sum+3];                       
		rtcmdata->rtcmbody[i].RRC=d1*(0.002+0.03*rtcmdata->rtcmbody[i].scale);		//伪距改正数的变化率

		rtcmdata->rtcmbody[i].IOD=words[sum+4];

		if (rtcmdata->rtcmbody[i].SatID > 0 && rtcmdata->rtcmbody[i].SatID <= 32 &&
			rtcmdata->rtcmbody[i].UDRE == 1)
			//UDRE=1时，为GPS卫星；UDRE=2时，为北斗卫星
		{
			n = rtcmdata->rtcmbody[i].SatID-1;
			satrtcm[n].IOD = rtcmdata->rtcmbody[i].IOD;
			satrtcm[n].RangeCorrection = (float)rtcmdata->rtcmbody[i].PRC0;
			satrtcm[n].RangeRateCorrection = (float)rtcmdata->rtcmbody[i].RRC;
			satrtcm[n].PRN = rtcmdata->rtcmbody[i].SatID;
			satrtcm[n].T = rtcmdata->zcount;
			satrtcm[n].status = true;
		}
		sum+=5;
	}
}

/*********************************************************************
DecodeRTCMBody

功能：解码这段RTCM电文的正文部分（N个字）

*********************************************************************/
void DecodeRTCMBody(BYTE buff[], int & k, int n, struct RTCMDATA *rtcmdata, int len, struct SATRTCM satrtcm[])
{
	int i,sum = 0;
	BYTE raw[250], rot[250], words[200];
	int satnum = (int)(0.6*len);
	short d2 = 0;
	char d1 = 0;

	for (i = 0; i < len; i++)			//读N*5个字节
	{
		for (int j = 0; j < 5;)
		{
			raw[i * 5 + j] = buff[k]; k++;
			if (j>=n)
			{
				return;
			}

			if ((raw[i * 5 + j] >> 7) == 0 && ((raw[i * 5 + j] >> 6) & 0x01) == 1)
			{
				rot[i * 5 + j] = RTCMRoll(raw[i * 5 + j]);
				j++;
			}
		}

		combine_to_word(&rot[i * 5], &words[i * 4]);
		if (ParityD30 == 1)  for (int j = 0; j < 3; j++) words[i * 4 + j] = ~words[i * 4 + j];

		if (RTCMParity(&words[i * 4]) == 0)		//奇偶校验失败
			return;
		ParityD30 = words[i * 4 + 3] & 0x01;
		ParityD29 = (words[i * 4 + 3] >> 1) & 0x01;
	}

	for (i = 2; i < 4 * len; i++)
	{
		if (fmod(i + 1, 4.0) == 0) {
			sum += 1;
			continue;
		}
		words[i - sum] = words[i];
	}

	sum = 0;
	for (i = 0; i < satnum; i++)
	{
		rtcmdata->rtcmbody[i].scale = (words[sum] >> 7) & 0x01;
		rtcmdata->rtcmbody[i].UDRE = (words[sum] >> 5) & 0x03;
		rtcmdata->rtcmbody[i].SatID = words[sum] & 0x1F;

		if (rtcmdata->rtcmbody[i].SatID == 0)
		{
			rtcmdata->rtcmbody[i].SatID = 32;//0表示的是32号卫星
		}

		d2 = words[sum + 1] << 8 | words[sum + 2];
		rtcmdata->rtcmbody[i].PRC0 = d2*(0.02 + 0.3*rtcmdata->rtcmbody[i].scale);		//伪距改正值
		d1 = words[sum + 3];
		rtcmdata->rtcmbody[i].RRC = d1*(0.002 + 0.03*rtcmdata->rtcmbody[i].scale);		//伪距改正数的变化率

		rtcmdata->rtcmbody[i].IOD = words[sum + 4];
		if (rtcmdata->rtcmbody[i].SatID > 0 && rtcmdata->rtcmbody[i].SatID <= 32 &&
			rtcmdata->rtcmbody[i].UDRE == 1)
			//UDRE=1时，为GPS卫星；UDRE=2时，为北斗卫星
		{
			n = rtcmdata->rtcmbody[i].SatID - 1;
			satrtcm[n].IOD = rtcmdata->rtcmbody[i].IOD;
			satrtcm[n].RangeCorrection = (float)rtcmdata->rtcmbody[i].PRC0;
			satrtcm[n].RangeRateCorrection = (float)rtcmdata->rtcmbody[i].RRC;
			satrtcm[n].PRN = rtcmdata->rtcmbody[i].SatID;
			satrtcm[n].T = rtcmdata->zcount;
			satrtcm[n].status = true;
		}
		sum += 5;
	}
}

/*********************************************************************
DecodeRTCM

功能：解码这段RTCM电文（N+2个字），输出解码结果satrtcm[32]，和rtcmdata

*********************************************************************/
void DecodeRTCM(FILE *fin, struct SATRTCM satrtcm[], struct RTCMDATA *rtcmdata)
{
	int i, j;

	BYTE raw[5],rot[5], words[4];                             
	j=0;
	do {
		if(feof(fin))
			break;
		for(i=0;i<5;j++)
		{
			fread(&raw[i], sizeof(BYTE), 1, fin);

			if((raw[i]>>7)==0&&((raw[i]>>6)&0x01)==1) 
			{
				rot[i]=RTCMRoll(raw[i]);
				i++;        
			}
		}

		combine_to_word(rot,words);

		findpreamble(fin, raw, words);

		if(DecodeRTCMHead(fin,raw,words,rtcmdata)==false)  
			continue;

		DecodeRTCMBody(fin, rtcmdata, rtcmdata->length, satrtcm);
		return;

		} while(!feof(fin));
}

/*********************************************************************
DecodeRTCM

参数：buff[]:接收到的rtcm电文数据
		n：接收到的电文数据的长度
		satrtcm[]：存放每颗卫星各自的rtcm改正数
		rtcmdata：解码实时的rtcm电文

功能：解码这段RTCM电文（N+2个字），输出解码结果satrtcm[32]，和rtcmdata

*********************************************************************/
void DecodeRTCM(BYTE buff[], int n, struct SATRTCM satrtcm[], struct RTCMDATA *rtcmdata)
{
	int i, j;

	BYTE raw[5], rot[5], words[4];
	j = 0;
	do {
		for (i = 0; i < 5; j++)
		{
			raw[i] = buff[j];

			if ((raw[i] >> 7) == 0 && ((raw[i] >> 6) & 0x01) == 1)
			{
				rot[i] = RTCMRoll(raw[i]);
				i++;
			}
		}

		combine_to_word(rot, words);

		findpreamble(buff, j, n, raw, words);

		if (DecodeRTCMHead(buff, j, n, raw, words, rtcmdata) == false)
			continue;

		DecodeRTCMBody(buff, j, n, rtcmdata, rtcmdata->length, satrtcm);
			break;

	} while (j<n);

}

/*********************************************************************
CalSatRTCM

功能：由当前的rtcmdata生成各个卫星的差分改正数结构体数组

*********************************************************************/
int CalSatRTCM(struct RTCMDATA *rtcmdata, struct SATRTCM satrtcm[])
{
	int i,n;
	int satnum=(int)(0.6*rtcmdata->length);
	for(i=0;i<satnum;i++)
	{
		if(rtcmdata->rtcmbody[i].SatID>0&&rtcmdata->rtcmbody[i].SatID<=32) 
		{
			n = rtcmdata->rtcmbody[i].SatID-1;
			satrtcm[n].IOD = rtcmdata->rtcmbody[i].IOD;
			satrtcm[n].RangeCorrection = (float)rtcmdata->rtcmbody[i].PRC0;
			satrtcm[n].RangeRateCorrection = (float)rtcmdata->rtcmbody[i].RRC;
			satrtcm[n].PRN = rtcmdata->rtcmbody[i].SatID;
			satrtcm[n].T = rtcmdata->zcount;
			satrtcm[n].status = true;
		}
	}
	return 0;
}
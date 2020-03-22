/*********************************************************************
��RTCM���ļ��ж�ȡ��ָ��������������ÿ�����ǵĲ�ָ�����

���ߣ�������
ʱ�䣺2017/6/7
*********************************************************************/
#include "stdafx.h"
#include "rtcm.h"
#include "struct.h"

BYTE rot[5];						//5*6λ��Ч��Ϣ
BYTE unite[4];						//24λ��Ч��Ϣ+6λ��żУ����Ϣ

static BYTE ParityD29=1;
static BYTE ParityD30=1;
/*********************************************************************
RTCMroll

���ܣ���8bit��unsigned char���ݽ���λ���㣬�õ��ֽ�ɨ��͹������6bit���ݣ�
	��6bit��������Ȼ����unsigned char������

������char kch��һ��8bit���ֽ�

����ֵ�����ص�6λ���ҹ����������

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

���ܣ���5��6bit�ֽ����ϳ�3��8bit�ֽں�һ��6bit�ֽ�

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

���ܣ������ҵ�ͬ����ʱ���ֽ���ҳ��

*********************************************************************/
void byte_jump_page(BYTE word[])
{
	word[0]=word[0]<<6|word[1]>>2;
	word[1]=word[1]<<6|word[2]>>2;
	word[2]=word[2]<<6|(word[3]&0x3F);
}
/*********************************************************************
RTCMParity

���ܣ�������żУ��

������const BYTE unite[]������������24λ+6λ��Ϣ
	BYTE rem����һ���ֵĺ���λ

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

���ܣ��ҵ������֣�01100110����λ�ã����������Ժ�ʼ����

*********************************************************************/
bool findpreamble(FILE* fin,BYTE raw[],BYTE w[])
{
	bool complement=0;		 //����

	do{
		if(ParityD30==1) 
		{
			for(int i=0;i<3;i++) w[i]=~w[i];
			complement=1;                                
		}

		if(w[0]==102)  return true;		//�ҵ�ͬ����

		if(complement==1) 
		{
			for(int i=0;i<3;i++) w[i]=~w[i];
			complement=0;                                
		}

		ParityD30=w[0]>>2&0x01;
		ParityD29=w[0]>>3&0x01;

		byte_jump_page(w);				//�ֽ���ҳ
		for(int i=0;i<4;i++)    
			raw[i]=raw[i+1];
		fread(&raw[4], sizeof(BYTE), 1, fin);

		w[3]=RTCMRoll(raw[4]);

	}while(!feof(fin));
	return true;
}

/*********************************************************************
���أ�findpreamble����buffer�н���rtcm���ģ�

������buff[]:���rtcm���ĵĻ�����
		j:�Ѷ����Ļ�������λ��
		n:rtcm���ĵĳ���

���ܣ��ҵ������֣�01100110����λ�ã����������Ժ�ʼ����

*********************************************************************/
bool findpreamble(BYTE buff[], int & j, int n, BYTE raw[], BYTE w[])
{
	bool complement = 0;		 //����

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
			return true;		//�ҵ�ͬ����
		}

		if (complement == 1)
		{
			for (int i = 0; i < 3; i++) w[i] = ~w[i];
			complement = 0;
		}

		ParityD30 = w[0] >> 2 & 0x01;
		ParityD29 = w[0] >> 3 & 0x01;

		byte_jump_page(w);				//�ֽ���ҳ
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

���ܣ��������RTCM���ĵ��ļ�ͷ�������֣�

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
		return false;  //У�����
	}

	rtcmdata->type=w[1]>>2;
	rtcmdata->id=((w[1]<<8)&0x0300)|w[2];

	ParityD29=w[3]>>1&0x01;  
	ParityD30=w[3]&0x01;

	for(int i=0;i<5;)								//��5���ֽ�
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
		return false;	    //У�����
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

���ܣ��������RTCM���ĵ��ļ�ͷ�������֣�

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
		return false;  //У�����
	}

	rtcmdata->type = w[1] >> 2;
	rtcmdata->id = ((w[1] << 8) & 0x0300) | w[2];

	ParityD29 = w[3] >> 1 & 0x01;
	ParityD30 = w[3] & 0x01;

	for (int i = 0; i < 5;)								//��5���ֽ�
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
		return false;	    //У�����
	}

	zcount = w[0] << 5 | ((w[1] >> 3) & 0x1F);

	rtcmdata->zcount = zcount*0.6;
	rtcmdata->seqno = w[1] & 0x07;
	//�ļ��У�����Ϊ5λ������״��Ϊ3λ
	//���ʵʱ���գ�����Ϊ6λ������״��Ϊ2λ
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

���ܣ��������RTCM���ĵ����Ĳ��֣�N���֣�

*********************************************************************/
void DecodeRTCMBody(FILE* fin, struct RTCMDATA *rtcmdata,int len, struct SATRTCM satrtcm[])
{
	int i, n,sum=0;
	BYTE raw[250], rot[250], words[200];
	int satnum=(int)(0.6*len);
	short d2=0;
	char d1=0;

	for(i=0;i<len;i++)			//��N*5���ֽ�
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

		if(RTCMParity(&words[i*4])==0)		//��żУ��ʧ��
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
			rtcmdata->rtcmbody[i].SatID == 32;//0��ʾ����32������
		}

		d2=words[sum+1]<<8|words[sum+2];		
		rtcmdata->rtcmbody[i].PRC0=d2*(0.02+0.3*rtcmdata->rtcmbody[i].scale);		//α�����ֵ
		d1=words[sum+3];                       
		rtcmdata->rtcmbody[i].RRC=d1*(0.002+0.03*rtcmdata->rtcmbody[i].scale);		//α��������ı仯��

		rtcmdata->rtcmbody[i].IOD=words[sum+4];

		if (rtcmdata->rtcmbody[i].SatID > 0 && rtcmdata->rtcmbody[i].SatID <= 32 &&
			rtcmdata->rtcmbody[i].UDRE == 1)
			//UDRE=1ʱ��ΪGPS���ǣ�UDRE=2ʱ��Ϊ��������
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

���ܣ��������RTCM���ĵ����Ĳ��֣�N���֣�

*********************************************************************/
void DecodeRTCMBody(BYTE buff[], int & k, int n, struct RTCMDATA *rtcmdata, int len, struct SATRTCM satrtcm[])
{
	int i,sum = 0;
	BYTE raw[250], rot[250], words[200];
	int satnum = (int)(0.6*len);
	short d2 = 0;
	char d1 = 0;

	for (i = 0; i < len; i++)			//��N*5���ֽ�
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

		if (RTCMParity(&words[i * 4]) == 0)		//��żУ��ʧ��
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
			rtcmdata->rtcmbody[i].SatID = 32;//0��ʾ����32������
		}

		d2 = words[sum + 1] << 8 | words[sum + 2];
		rtcmdata->rtcmbody[i].PRC0 = d2*(0.02 + 0.3*rtcmdata->rtcmbody[i].scale);		//α�����ֵ
		d1 = words[sum + 3];
		rtcmdata->rtcmbody[i].RRC = d1*(0.002 + 0.03*rtcmdata->rtcmbody[i].scale);		//α��������ı仯��

		rtcmdata->rtcmbody[i].IOD = words[sum + 4];
		if (rtcmdata->rtcmbody[i].SatID > 0 && rtcmdata->rtcmbody[i].SatID <= 32 &&
			rtcmdata->rtcmbody[i].UDRE == 1)
			//UDRE=1ʱ��ΪGPS���ǣ�UDRE=2ʱ��Ϊ��������
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

���ܣ��������RTCM���ģ�N+2���֣������������satrtcm[32]����rtcmdata

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

������buff[]:���յ���rtcm��������
		n�����յ��ĵ������ݵĳ���
		satrtcm[]�����ÿ�����Ǹ��Ե�rtcm������
		rtcmdata������ʵʱ��rtcm����

���ܣ��������RTCM���ģ�N+2���֣������������satrtcm[32]����rtcmdata

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

���ܣ��ɵ�ǰ��rtcmdata���ɸ������ǵĲ�ָ������ṹ������

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
/*********************************************************************
�ӻ�õ���������ȡ��Ҫ����������̰���CRCУ��͸���������������ȡ�������
����������ȡ�������ǹ۲�������ȡ����λ������ȡ


���ߣ�������
ʱ�䣺2017/5/8
*********************************************************************/
#pragma once
#ifndef _DECODE_H
#define _DECODE_H

#include <stdio.h>
#include <stdlib.h>

int GetDataFromSerial(unsigned char * buff, unsigned char * databuffer, const int & length, int & validlen, int flag);
unsigned int crc32(const unsigned char *buff, int len);
int checkdata(const unsigned int cal_crc, const unsigned char *buff, int len);
int getdata(const unsigned char *buff, struct RANGE *, struct GPSEPHEM [], struct IONUTC *, struct PSRPOS *);


#endif
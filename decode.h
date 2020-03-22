/*********************************************************************
从获得的数据中提取需要的数据项，过程包括CRC校验和各卫星星历数据提取、电离层
改正数据提取、各卫星观测数据提取、定位数据提取


作者：王雅仪
时间：2017/5/8
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
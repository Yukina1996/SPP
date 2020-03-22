/*********************************************************************
从RTCM的文件中读取差分改正数，单独存放每颗卫星的差分改正数

作者：王雅仪
时间：2017/6/7
*********************************************************************/
#pragma once
#ifndef _RTCM_H
#define _RTCM_H
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define   BYTE  unsigned char

BYTE RTCMRoll(BYTE kch);
void combine_to_word(BYTE rot[], BYTE words[]);
void byte_jump_page(BYTE w[]);
bool findpreamble(FILE* fin,BYTE raw[],BYTE w[]);
bool findpreamble(BYTE buff[], int & j, int n, BYTE raw[], BYTE w[]);
bool RTCMParity(BYTE w[]);
bool DecodeRTCMHead(FILE *fin, BYTE raw[],BYTE w[],struct RTCMDATA *rtcmdata);
bool DecodeRTCMHead(BYTE buff[], int & j, int n, BYTE raw[], BYTE w[], struct RTCMDATA *rtcmdata);
void DecodeRTCMBody(FILE* fin, struct RTCMDATA *rtcmdata,int len, struct SATRTCM satrtcm[]);
void DecodeRTCMBody(BYTE buff[], int & j, int n, struct RTCMDATA *rtcmdata, int len, struct SATRTCM satrtcm[]);
void DecodeRTCM(FILE *fin, struct SATRTCM satrtcm[], struct RTCMDATA *rtcmdata);
void DecodeRTCM(BYTE buff[],int n, struct SATRTCM satrtcm[], struct RTCMDATA *rtcmdata);
int CalSatRTCM(struct RTCMDATA *rtcmdata, struct SATRTCM satrtcm[]);

#endif

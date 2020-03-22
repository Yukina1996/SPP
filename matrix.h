/**********************************************
����\������ؼ���ĺ���ģ��

���ߣ�������
ʱ�䣺2017.04.25
***********************************************/
#pragma once
#ifndef _MATRIX_H
#define _MATRIX_H

//ͷ�ļ�    
#include <stdio.h>    
#include <stdlib.h>   

bool matrix_add(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[]);
bool matrix_minus(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[]);
bool matrix_multiply(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[]);
bool matrix_transfer(const int row1, const int col1, const double a[], double b[]);
bool matrix_inverse(int n, double a[], double b[]);

double vector_dot(const int row1, const int col1, const int row2, const int col2, const double a[],
	const double b[]);
bool vector_cross(const int row1, const int col1, const int row2, const int col2, const double a[],
	const double b[], double c[]);


#endif
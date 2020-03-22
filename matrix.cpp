/**********************************************
����������ؼ���ĺ���ģ��

���ߣ�������
ʱ�䣺2017.04.25
***********************************************/
#include "stdafx.h"
#include <math.h>
#include "matrix.h"
#include <iostream>
#include <cfloat>
using namespace std;

/**********************************************
matrix_add

���ܣ�ʵ�־����������,c=a+b

������row1,col1�Ǿ���a[]���к���
      row2,col2�Ǿ���b[]���к���

***********************************************/
bool matrix_add(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[])
{
	int i,j;

	if(row1!=row2 || col1!=col2)
	{
		printf("������ӵ����������в���");
		return false;
	}

	for(i=0; i<row1; i++)
	{
		for(j=0; j<col1; j++)
		{
			c[i*col1+j]=a[i*col1+j]+b[i*col1+j];
		}
	}
	return true;

}

/**********************************************
matrix_minus

���ܣ�ʵ�־����������,c=a-b

������row1,col1�Ǿ���a[]���к���
      row2,col2�Ǿ���b[]���к���

***********************************************/
bool matrix_minus(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[])
{
	int i, j;

	if(row1!=row2 || col1!=col2)
	{
		printf("������������������в���");
		return false;
	}

	for(i=0; i<row1; i++)
	{
		for(j=0; j<col1; j++)
		{
			c[i*col1+j]=a[i*col1+j]-b[i*col1+j];
		}
	}
	return true;
}

/**********************************************
matrix_multiply

���ܣ�ʵ�־����������,c=a*b

������row1,col1�Ǿ���a[]���к���
      row2,col2�Ǿ���b[]���к���

***********************************************/
bool matrix_multiply(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[])
{
	int i, j, k;
	double val;

	if(col1!=row2)
	{
		printf("���ܽ��о�����ˣ���Ϊǰ��������������ں���������");
		return false;
	}

	for(i=0; i<row1; i++)
	{
		for(j=0; j<col2; j++)
		{
			val = 0.0;
			for(k=0; k<col1; k++)
			{
				val += a[i*col1+k] * b[k*col2+j];
			}
			c[i*col2+j]=val;
		}
	}
	return true;
}

/**********************************************
matrix_transfer

���ܣ�ʵ�־���ת�����㣬a����ת�ó�Ϊb����

������row1,col1�Ǿ���a[]���к���
      row2,col2�Ǿ���b[]���к���

***********************************************/
bool matrix_transfer(const int row1, const int col1, const double a[], double b[])
{
	int row2, col2;
	row2=col1;//b�����������ԭa���������
	col2=row1;//b�����������ԭa���������
	int i, j;
	 
	for(i=0; i<row1; i++)
	{
		for(j=0; j<col1; j++)
		{
			b[j*col2+i]=a[i*col1+j];
		}
	}
	return true;
}

/**********************************************
matrix_inverse

���ܣ�ʵ�־����������㣬a����ת�ó�Ϊb����

������n�Ǿ���a�ľ���ά����Ҳ���Ǿ���a����
      ����a�������Ϊb

***********************************************/
bool matrix_inverse(int n, double a[], double b[] )
{
	int i,j,k,l,u,v,is[10],js[10];   /* matrix dimension <= 10 */
	double d, p;

	if( n<= 0 )
	{
		printf( "Error dimension in MatrixInv!\n");
		return false;
	}

	/* ���������ֵ���������b�������b�������棬a���󲻱� */
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			b[i*n+j]=a[i*n+j];
		}
	}

	for(k=0;k<n;k++)
	{
		d=0.0;
		for(i=k;i<n;i++)   /* �������½Ƿ�������Ԫ�ص�λ�� */
		{
			for(j=k;j<n;j++)
			{
				l=n*i+j;
				p = fabs(b[l]);
				if(p>d)
				{
					d=p;
					is[k]=i;
					js[k]=j;
				}
			}
		}

		if(d<DBL_EPSILON)   /* ��Ԫ�ؽӽ���0�����󲻿��� */
		{
			printf("Divided by 0 in MatrixInv!\n");
			return false;
		}

		if( is[k]!=k )  /* ����Ԫ�����ڵ��������½Ƿ�������н��е��� */
		{
			for(j=0;j<n;j++)
			{
				u=k*n+j;
				v=is[k]*n+j;
				p=b[u];
				b[u]=b[v];
				b[v]=p;
			}
		}

		if( js[k]!=k )  /* ����Ԫ�����ڵ��������½Ƿ�������н��е��� */
		{
			for( i=0; i<n; i++ )
			{
				u=i*n+k;
				v=i*n+js[k];
				p=b[u];
				b[u]=b[v];
				b[v]=p;
			}
		}

		l=k*n+k;
		b[l]=1.0/b[l];  /* �����б任 */
		for( j=0; j<n; j++ )
		{
			if( j!=k )
			{
				u=k*n+j;
				b[u]=b[u]*b[l];
			}
		}
		for(i=0;i<n; i++)
		{
			if(i!=k)
			{
				for(j=0; j<n; j++ )
				{
					if( j!=k )
					{
						u=i*n+j;
						b[u]=b[u]-b[i*n+k]*b[k*n+j];
					}
				}
			}
		}
		for(i=0;i<n;i++)
		{
			if(i!=k)
			{
				u=i*n+k;
				b[u]=-b[u]*b[l];
			}
		}
	}

	for(k=n-1;k>=0;k--)  /* ����������е������»ָ� */
	{
		if(js[k]!=k)
		{
			for(j=0;j<n;j++)
			{
				u=k*n+j;
				v=js[k]*n+j;
				p=b[u];
				b[u]=b[v];
				b[v]=p;
			}
		}
		if(is[k]!=k)
		{
			for(i=0;i<n;i++)
			{
				u=i*n+k;
				v=is[k]+i*n;
				p=b[u];
				b[u]=b[v];
				b[v]=p;
			}
		}
	}

	return true;
}

/**********************************************
vector_dot

���ܣ�ʵ��ʸ���ĵ�����㣬ʸ��������ľ���
      ����һ�����еľ���Ϊʸ��

������row1��ʸ��a���У�col1��ʸ��a����
     row2��ʸ��b���У�col2��ʸ��b����
	  row1��row2����Ϊ1��col1��col2����Ϊ3
	 ʸ��a,b�ĵ�����Ϊc��c��һ��ʵ��ֵ

***********************************************/
double vector_dot(const int row1, const int col1, const int row2, const int col2, const double a[],
	const double b[])
{
	if(row1!=1 || row2!=1 || col1!=3 || col2!=3)
	{
		printf("�����˷Ƿ�ʸ��");
		return 0;
	}

	double c=0.0;
	int i;
	for(i=0; i<3; i++)
	{
		c += a[i]*b[i];
	}

	return c;
}

/**********************************************
vector_cross

���ܣ�ʵ��ʸ���Ĳ�����㣬ʸ��������ľ���
      ����һ�����еľ���Ϊʸ��

������row1��ʸ��a���У�col1��ʸ��a����
     row2��ʸ��b���У�col2��ʸ��b����
	 row1��row2����Ϊ1��col1��col2����Ϊ3
	 ʸ��a,b�Ĳ�����Ϊc��cҲ��һ��ʸ��

***********************************************/
bool vector_cross(const int row1, const int col1, const int row2, const int col2, const double a[],
	const double b[], double c[])
{
	if(row1!=1 || row2!=1 || col1!=3 || col2!=3)
	{
		printf("�����˷Ƿ�ʸ��");
		return false;
	}

	c[0]=a[1]*b[2]-b[1]*a[2];
	c[1]=a[0]*b[2]-b[0]*a[2];
	c[2]=a[0]*b[1]-b[0]*a[1];

	return true;
}
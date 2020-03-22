/**********************************************
矩阵向量相关计算的函数模块

作者：王雅仪
时间：2017.04.25
***********************************************/
#include "stdafx.h"
#include <math.h>
#include "matrix.h"
#include <iostream>
#include <cfloat>
using namespace std;

/**********************************************
matrix_add

功能：实现矩阵相加运算,c=a+b

参数：row1,col1是矩阵a[]的行和列
      row2,col2是矩阵b[]的行和列

***********************************************/
bool matrix_add(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[])
{
	int i,j;

	if(row1!=row2 || col1!=col2)
	{
		printf("出错，相加的两矩阵行列不等");
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

功能：实现矩阵相减运算,c=a-b

参数：row1,col1是矩阵a[]的行和列
      row2,col2是矩阵b[]的行和列

***********************************************/
bool matrix_minus(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[])
{
	int i, j;

	if(row1!=row2 || col1!=col2)
	{
		printf("出错，相减的两矩阵行列不等");
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

功能：实现矩阵相乘运算,c=a*b

参数：row1,col1是矩阵a[]的行和列
      row2,col2是矩阵b[]的行和列

***********************************************/
bool matrix_multiply(const int row1, const int col1, const int row2, const int col2, const double a[], 
	const double b[], double c[])
{
	int i, j, k;
	double val;

	if(col1!=row2)
	{
		printf("不能进行矩阵相乘，因为前矩阵的列数不等于后矩阵的行数");
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

功能：实现矩阵转置运算，a矩阵转置成为b矩阵

参数：row1,col1是矩阵a[]的行和列
      row2,col2是矩阵b[]的行和列

***********************************************/
bool matrix_transfer(const int row1, const int col1, const double a[], double b[])
{
	int row2, col2;
	row2=col1;//b矩阵的列数是原a矩阵的行数
	col2=row1;//b矩阵的行数是原a矩阵的列数
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

功能：实现矩阵求逆运算，a矩阵转置成为b矩阵

参数：n是矩阵a的矩阵维数，也就是矩阵a的秩
      矩阵a的逆矩阵为b

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

	/* 将输入矩阵赋值给输出矩阵b，下面对b矩阵求逆，a矩阵不变 */
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
		for(i=k;i<n;i++)   /* 查找右下角方阵中主元素的位置 */
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

		if(d<DBL_EPSILON)   /* 主元素接近于0，矩阵不可逆 */
		{
			printf("Divided by 0 in MatrixInv!\n");
			return false;
		}

		if( is[k]!=k )  /* 对主元素所在的行与右下角方阵的首行进行调换 */
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

		if( js[k]!=k )  /* 对主元素所在的列与右下角方阵的首列进行调换 */
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
		b[l]=1.0/b[l];  /* 初等行变换 */
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

	for(k=n-1;k>=0;k--)  /* 将上面的行列调换重新恢复 */
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

功能：实现矢量的点积运算，矢量是特殊的矩阵
      考虑一行三列的矩阵为矢量

参数：row1是矢量a的行，col1是矢量a的列
     row2是矢量b的行，col2是矢量b的列
	  row1、row2必须为1，col1、col2必须为3
	 矢量a,b的点积结果为c，c是一个实数值

***********************************************/
double vector_dot(const int row1, const int col1, const int row2, const int col2, const double a[],
	const double b[])
{
	if(row1!=1 || row2!=1 || col1!=3 || col2!=3)
	{
		printf("输入了非法矢量");
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

功能：实现矢量的叉积运算，矢量是特殊的矩阵
      考虑一行三列的矩阵为矢量

参数：row1是矢量a的行，col1是矢量a的列
     row2是矢量b的行，col2是矢量b的列
	 row1、row2必须为1，col1、col2必须为3
	 矢量a,b的叉积结果为c，c也是一个矢量

***********************************************/
bool vector_cross(const int row1, const int col1, const int row2, const int col2, const double a[],
	const double b[], double c[])
{
	if(row1!=1 || row2!=1 || col1!=3 || col2!=3)
	{
		printf("输入了非法矢量");
		return false;
	}

	c[0]=a[1]*b[2]-b[1]*a[2];
	c[1]=a[0]*b[2]-b[0]*a[2];
	c[2]=a[0]*b[1]-b[0]*a[1];

	return true;
}
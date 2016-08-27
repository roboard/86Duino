/*
  Cubic.cpp - The part of Servo library
  
  Copyright (c) Designer 
    2016 Sayter <sayter@dmp.com.tw>. All right reserved.
    2016 AAA <aaa@dmp.com.tw>. All right reserved.
  Copyright (c) Programmer
    2016 Android Lin <acen@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#include "Cubic.h"

double** mallocMyDoubleMatrix(int m, int n) {
	double **Array, *pData;
	int i;
	Array = (double**) malloc(m*sizeof(double *));
	pData = (double*) malloc(m*n*sizeof(double));
	for (i = 0; i < m; i++, pData += n)
		Array[i] = pData;
	return Array;
}

void freeMyDoubleMatrix(double** m, double* n) {
	free(n);
	free(m);
}

void servoComputeCubicSpline (double** points, double** acc, unsigned long size) {
	int totalPoints = 0;
	int totalLines = 0;

	double* x_Array;
	double* h_Array;
	double* L_Array;
	double* U_Array;
	double* z_Array;
	double* alpha_Array;
	//double **result_Matrix;
	int i, j;

	if (points == NULL) return;
	if (size == 1) {acc[0][0] = 0.0; acc[0][1] = 0.0; return;}
	if (size == 2) {acc[0][0] = 0.0; acc[0][1] = 0.0; acc[1][0] = 0.0; acc[1][1] = 0.0; return;}
	
	totalPoints = size;
	totalLines = totalPoints - 1L;
	
	h_Array = (double*) malloc(totalLines*6*sizeof(double));
	x_Array = h_Array + totalLines;
	alpha_Array = x_Array + totalLines;
	L_Array =  alpha_Array + totalLines;
	U_Array =  L_Array + totalLines;
	z_Array =  U_Array + totalLines;
	
	for(i=0; i<totalLines; i++)
		h_Array[i] = points[i+1][0] - points[i][0];

	// initialize X (C0 ~ Cn)
	for (i=0; i<totalPoints; i++)
		x_Array[i] = 0.0;

	//result_Matrix = mallocMyDoubleArray((int)totalLines, (int)totalPoints); // totalLines = totalPoints - 1

	for (i=1; i<totalLines; i++)
		alpha_Array[i] = (3*(points[i+1][1] - points[i][1])/h_Array[i]) - (3*(points[i][1] - points[i-1][1])/h_Array[i-1]);

	for (i=0; i<totalPoints; i++)
	{
		if (i == 0 || i == totalLines)
		{
			L_Array[i] = 1.0; U_Array[0] = 0.0; z_Array[0] = 0.0;
		}
		else
		{
			L_Array[i] = 2*(points[i+1][0] - points[i-1][0]) - h_Array[i-1]*U_Array[i-1];
			U_Array[i] = h_Array[i]/L_Array[i];
			z_Array[i] = (alpha_Array[i] - h_Array[i-1]*z_Array[i-1])/L_Array[i];
		}
	}

	for (i=totalLines-1; i>=0; i--)
	{
		x_Array[i] = z_Array[i] - U_Array[i]*z_Array[i+1];
		//result_Matrix[i][1] = ((points[i+1][1] - points[i][1])/h_Array[i]) - (h_Array[i]*(x_Array[i+1] + 2*x_Array[i])/3); // caculate b
		//result_Matrix[i][3] = (x_Array[i+1] - x_Array[i])/(3*h_Array[i]); // caculate d
		acc[i][0] = acc[i][1] = 2*x_Array[i];
	}
	acc[totalLines][0] = 0; // the speed of last point is 0
	acc[totalLines][1] = 0;
	
	//for (i=0; i<totalLines; i++) result_Matrix[i][0] = points[i][1]; // caculate a
	//for (i=0; i<totalLines; i++) result_Matrix[i][2] = x_Array[i]; // caculate c

	free(h_Array);
}

void servoComputeConstrainedCubicSpline (double** points, double** acc, unsigned long size) {
	int totalPoints = 0;
	int totalLines = 0;

	double* dx;
	double* dy;
	double* pointSpeed;
	double* myc;
	double* myd;
	int i, j;

	if (points == NULL) return;
	if (size == 1) {acc[0][0] = 0.0; acc[0][1] = 0.0; return;}
	if (size == 2) {acc[0][0] = 0.0; acc[0][1] = 0.0; acc[1][0] = 0.0; acc[1][1] = 0.0; return;}
	
	totalPoints = size;
	totalLines = totalPoints - 1L;
	
	dx = (double*) malloc(totalLines*5*sizeof(double));
	dy = dx + totalLines;
	pointSpeed = dy + totalLines;
	myc = pointSpeed + totalLines;
	myd = myc + totalLines;
	
	for (i=0; i<totalLines; i++)
	{
		dx[i] = points[i+1][0] - points[i][0];
		if (dx[i] <= 20.0) dx[i] = 20.0;
		dy[i] = points[i+1][1] - points[i][1];
	}
	
	for (i=1; i<totalLines; i++)
	{
		long S = (long) (dy[i-1]*dy[i]);
		if (S > 0L)
			pointSpeed[i] = 2.0/((dx[i]/dy[i]) + (dx[i-1]/dy[i-1]));
		else
			pointSpeed[i] = 0.0;
	}
	
	pointSpeed[0] = ((3.0*dy[0])/(2.0*dx[0])) - (pointSpeed[1]/2.0);
	pointSpeed[totalLines] = ((3.0*dy[totalLines-1])/(2.0*dx[totalLines-1])) - (pointSpeed[totalLines-1]/2.0);
	
	for (i=1; i<=totalLines; i++)
	{
		double lacc = ((6.0*dy[i-1])/(dx[i-1]*dx[i-1])) - ((2.0*(pointSpeed[i] + 2.0*pointSpeed[i-1]))/dx[i-1]);
		double racc = ((2.0*(2.0*pointSpeed[i] + pointSpeed[i-1]))/dx[i-1]) - ((6.0*dy[i-1])/(dx[i-1]*dx[i-1]));
		
		myd[i-1] = (racc - lacc)/(6.0*dx[i-1]);
		myc[i-1] = ((points[i][0]*lacc) - (points[i-1][0]*racc))/(2.0*dx[i-1]);
		
		acc[i-1][1] = 2.0*myc[i-1] + 6.0*myd[i-1]*points[i-1][0];
		acc[i][0] = 2.0*myc[i-1] + 6.0*myd[i-1]*points[i][0];
	}

	acc[0][0] = 0.0;
	acc[i-1][1] = 0.0;
	
	/*
	printf("d: ");
	for (i=0; i<totalLines; i++)
		printf("%f ", myd[i]);
	printf("\n");
	
	printf("c: ");
	for (i=0; i<totalLines; i++)
		printf("%f ", myc[i]);
	printf("\n");
	*/
	
	free(dx);
}

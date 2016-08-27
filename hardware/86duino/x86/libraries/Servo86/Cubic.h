#ifndef __CUBIC_h
#define __CUBIC_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double** mallocMyDoubleMatrix(int m, int n);
void freeMyDoubleMatrix(double** m, double* n);
void servoComputeCubicSpline (double** points, double** acc, unsigned long _size);
void servoComputeConstrainedCubicSpline (double** points, double** acc, unsigned long _size);

#endif


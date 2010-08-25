/* TNC : Minimization example */
/* $Jeannot: example.c,v 1.19 2005/01/28 18:27:31 js Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tnc.h"

static tnc_function function;

double* xopt;

static double calc_E(double x[]) {
	double sum = 0; int i=0,j; double xi_xj;
	for(;i<5;i++) {
		xi_xj = (x[i] - xopt[i]);
		sum += xi_xj*xi_xj;
/*		if(i==0) {
			sum += (xi_xj - x[1]-xopt[1])*(xi_xj - x[1]-xopt[1]);
		} else if(i==4) {
			sum += (xi_xj - x[3]-xopt[3])*(xi_xj - x[3]-xopt[3]);
		} else */{
			for(j=-1;j<2;j+=2) {
				if(i==0 && j==-1) continue;
				if(i==4 && j==1) continue;
				if(i+j==i) continue;
				sum += ((xi_xj - x[i+j]-xopt[i+j])*(xi_xj - x[i+j]-xopt[i+j]));
			}
		}
	}
	return sum;
}

#define EPSILON 0.00001

static int function(double x[], double *f, double g[], void *state)
{
	int i; int j; double E_x_epsilon; double x_epsilon[5];
	*f = calc_E(x);

	for(i=0;i<5;i++) {
		for(j=0;j<5;j++) {
			x_epsilon[j] = x[j];
			if(j==i) x_epsilon[j] += EPSILON;
		}
		E_x_epsilon = calc_E(x_epsilon);

		g[i] = (E_x_epsilon - *f) / EPSILON;
	}
	//g[0] = 2.0*x[0];
	//g[1] = 3.0*pow(fabs(x[1]),2.0);
	//if(x[1]<0) g[1] = -g[1];
	return 0;
}

int simple_tnc(int n, double x[], /*double _xopt[],*/ double *f, double g[], tnc_function *function, void* state, double ubound1, double ubound2) {
  int i, rc, maxCGit = 5, maxnfeval = 350, nfeval;
  double fopt = 1.0,
	*low, *up,
    eta = -1.0, stepmx = 10.0,
    accuracy = -1.0, fmin = 0.0, ftol = -1.0, xtol = -1.0, pgtol = -1.0,
    rescale = -1.0, maxv = -1.0;

  //xopt = (double*)malloc(sizeof(double)*n);
  //for(i=0;i<n;i++) xopt[i] = _xopt[i];

  up = (double*)malloc(sizeof(double)*n);
  low = (double*)malloc(sizeof(double)*n);

  //for(i=0;i<n;i++) { if(x[i] > maxv) maxv = x[i]; }
  for(i=0;i<n;i++) { up[i] = ((i%2==0)?ubound1:ubound2); low[i] = 2.0; }

  rc = tnc(n, x, f, g, function, state, low, up, NULL, NULL, TNC_MSG_ALL,
    maxCGit, maxnfeval, eta, stepmx, accuracy, fmin, ftol, xtol, pgtol,
    rescale, &nfeval);

  free(up);
  free(low);

  return rc;
}

int tnc_main(int argc, char **argv)
{
  int i, rc, maxCGit = 2, maxnfeval = 200, nfeval;
  double fopt = 1.0, f, g[5],
    x[5] = {0.0,0.0,0.0,0.0,0.0},
	low[5] = {0.0,0.0,0.0,0.0,0.0}, up[5] = {10.0,10.0,10.0,10.0,10.0},
    eta = -1.0, stepmx = 20.0,
    accuracy = -1.0, fmin = 0.0, ftol = -1.0, xtol = -1.0, pgtol = -1.0,
    rescale = -1.0;

  //low[0] = - HUGE_VAL; low[1] = 1.0;
  //up[0] = HUGE_VAL; up[1] = HUGE_VAL;

  rc = tnc(5, x, &f, g, function, NULL, low, up, NULL, NULL, TNC_MSG_ALL,
    maxCGit, maxnfeval, eta, stepmx, accuracy, fmin, ftol, xtol, pgtol,
    rescale, &nfeval);

  printf("After %d function evaluations, TNC returned:\n%s\n", nfeval,
    tnc_rc_string[rc - TNC_MINRC]);

  for (i = 0; i < 5; i++)
    printf("x[%d] = %.5f / xopt[%d] = %.5f\n", i, x[i], i, xopt[i]);

  printf("\n");
  printf("f    = %.15f / fopt    = %.15f\n", f, fopt);
	scanf(" ");
  return 0;
}

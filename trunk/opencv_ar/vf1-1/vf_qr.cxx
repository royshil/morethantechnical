#include "vf_qr.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

// declare LAPACK routine.
typedef int integer;
typedef double doublereal;
extern "C" {
  int dgeqp3_(integer *m,
              integer *n,
              doublereal *a,
              integer *lda,
              integer *jpvt,
              doublereal *tau,
              doublereal *work,
              integer *lwork,
              integer *info);
};

#ifndef var_size_arrays
# if defined(__GNUC__)
#  define var_size_arrays 1
# else
#  define var_size_arrays 0
# endif
#endif

void vf_qr(int m, int n, double const *A, double *R, int p[])
{
#if var_size_arrays
  int const tmp_space = m*n;
  int const tau_space = (m < n) ? m : n;
  int const work_space = 3*n + 1;
#else
  int const mmax = 6; assert(m <= mmax);
  int const nmax = 9; assert(n <= nmax);
  int const tmp_space = mmax*nmax;
  int const tau_space = (mmax < nmax) ? mmax : nmax;
  int const work_space = 3*nmax + 1;
#endif
  
  // copy C to FORTRAN.
  double tmp[tmp_space];
  for (int i=0; i<m; ++i)
    for (int j=0; j<n; ++j)
      tmp[i + m*j] = A[n*i + j];
  
  for (int j=0; j<n; ++j)
    p[j] = 0; // "free" column.
  
  double tau[tau_space];
  double work[work_space];
  
  int info;
  {
    int tmp_lwork = work_space;
    dgeqp3_(&m,          // integer *m,
            &n,          // integer *n,
            tmp,         // doublereal *a,
            &m,          // integer *lda,
            p,           // integer *jpvt,
            tau,         // doublereal *tau,
            work,        // doublereal *work,
            &tmp_lwork,  // integer *lwork,
            &info);      // integer *info);
  }
  if (info != 0) {
    std::fprintf(stderr, __FILE__ ": info = %d\n", info);
    std::fprintf(stderr, __FILE__ ": %dth argument has illegal value\n", -info);
    std::abort();
  }
  
  // adjust p.
  for (int j=0; j<n; ++j)
    -- p[j];
  
#define A(i, j) (tmp[(i) + m * (j)])
#define R(i, j) (R[n * (i) + (j)])
  // fill in R.
  for (int i=0; i<m; ++i) {
    for (int j=0; j<i; ++j)
      R(i, j) = 0.0;
    for (int j=i; j<n; ++j)
      R(i, p[j]) = A(i, j);
  }
#undef A
#undef R
}

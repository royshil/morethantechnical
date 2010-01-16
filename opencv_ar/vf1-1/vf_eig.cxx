#include "vf_eig.h"

//#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>

// declare LAPACK routine.
typedef int integer;
typedef double doublereal;
extern "C" {
  int dgeev_(char const *jobvl,
             char const *jobvr,
             integer *n,
             doublereal *a,
             integer *lda,
             doublereal *wr,
             doublereal *wi,
             doublereal *vl,
             integer *ldvl,
             doublereal *vr,
             integer *ldvr,
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

void vf_eig(int N, double const *A, double *Dreal, double *Dimag, double *Vreal, double *Vimag)
{
#if var_size_arrays
  int const tmp_space = N*N;
  int const work_space = 10*N;
  int const Rz_space = N*N;
#else
  int const Nmax = 15; assert(N <= Nmax);
  int const tmp_space = Nmax*Nmax;
  int const work_space = 10*Nmax;
  int const Rz_space = Nmax*Nmax;
#endif
  
  // copy A.
  double tmp[tmp_space];
  for (int i=0; i<N*N; ++i) {
    tmp[i] = A[i];
    //assert(finite(tmp[i]));
  }
  
  double work[work_space];
  
  // result stored here.
  double Rz[Rz_space];
  
  int info;
  {
    int tmp_N = N;
    int tmp_lwork = work_space;
    dgeev_("V",                        // jobvl
           "N",                        // jobvr
           &tmp_N,                     // n
           tmp,                        // a
           &tmp_N,                     // lda
           Dreal,                      // wr
           Dimag,                      // wi
           Rz,                         // vl
           &tmp_N,                     // ldvl
           0,                          // vr
           &tmp_N,                     // ldvr
           work,                       // work
           &tmp_lwork,                 // lwork
           &info                       // info
           );
    assert(tmp_N == int(N));
  }
  
  if (info != 0) {
    std::fprintf(stderr, __FILE__ ": info = %d\n", info);
    if (info < 0)
      std::fprintf(stderr, __FILE__ ": %dth argument has illegal value\n", -info);
    else
      std::fprintf(stderr, __FILE__ ": QR algorithm failed to compute all eigenvalues.\n");
    std::abort();
  }
  
  // need to conjugate and transpose R for C convention.
#define Vreal(i, j) (Vreal[N * (i) + (j)])
#define Vimag(i, j) (Vimag[N * (i) + (j)])
#define    Rz(i, j) (   Rz[N * (i) + (j)])
  for (int k=0; k<N; ) {
    if (Dimag[k] == 0.0) {
      // real eigenvalue.
      for (int i=0; i<N; ++i) {
        Vreal(i, k) = Rz(k, i);
        Vimag(i, k) = 0.0;
      }
      ++ k;
    }
    else {
      // complex eigenvalue.
      assert(k+1 < N);
      for (int i=0; i<N; ++i) {
        Vreal(i, k+0) =  Rz(k+0, i);
        Vimag(i, k+0) = -Rz(k+1, i);
        
        Vreal(i, k+1) =  Rz(k+0, i);
        Vimag(i, k+1) = +Rz(k+1, i);
      }
      ++ k;
      ++ k;
    }
  }
#undef Vreal
#undef Vimag
#undef Rz
}

#include "vf_solve.h"

#include <cmath>
#include <cstdio>
#include <cassert>

#include "vf_qr.h"
#include "vf_10_33.h"
#include "vf_eig.h"

#if 0
static
void L_one_normalize(int n, double *v)
{
  double t = 0.0;
  for (int i=0; i<n; ++i)
    t += std::abs(v[i]);
  
  if (t > 0.0) {
    for (int i=0; i<n; ++i)
      v[i] /= t;
  }
}
#endif

#if 0
static
void L_two_normalize(int n, double *v)
{
  double t = 0.0;
  for (int i=0; i<n; ++i)
    t += v[i]*v[i];
  
  if (t > 0.0) {
    t = std::sqrt(t);
    for (int i=0; i<n; ++i)
      v[i] /= t;
  }
}
#endif

#if 1
static
void L_inf_normalize(int n, double *v)
{
  double t = 0.0;
  for (int i=0; i<n; ++i) {
    double tmp = std::abs(v[i]);
    if (tmp > t)
      t = tmp;
  }
  
  if (t > 0.0) {
    for (int i=0; i<n; ++i)
      v[i] /= t;
  }
}
#endif

static
void normalize(int n, double *v)
{
  //L_one_normalize(n, v);
  //L_two_normalize(n, v);
  L_inf_normalize(n, v);
}

#if 0
// nicely print m x n matrix in MATLAB format.
static
void print(std::FILE *fp, char const *name, int m, int n, double const *A)
{
  char const *format = " %+e";
  //char const *format = " %+6.4f";
  
  std::fprintf(stderr, "%s = [ ...\n", name);
  for (int i=0; i<m; ++i) {
    for (int j=0; j<n; ++j)
      std::fprintf(stderr, format, A[n*i + j]);
    std::fprintf(stderr, "\n");
  }
  std::fprintf(stderr, "];\n");
}
#endif

// Multiplication by p. No checking for unwanted non-zeros.
static
void times_p(double const u[33], double v[33])
{
  // shifter = [NaN 1 2 NaN NaN NaN 4 5 6 7 NaN NaN NaN NaN 8 9 12 NaN 11 13 14 18 NaN 17 19 20 21 22 23 27 28 29 32]
  
  // for j=1:33; i = find(shifter == j); if isempty(i); fprintf(1, '  v[%2d] = 0.0;\n', j-1); else fprintf(1, '  v[%2d] = u[%2d];\n', j-1, i-1); end; end
  v[ 0] = u[ 1];
  v[ 1] = u[ 2];
  v[ 2] = 0.0;
  v[ 3] = u[ 6];
  v[ 4] = u[ 7];
  v[ 5] = u[ 8];
  v[ 6] = u[ 9];
  v[ 7] = u[14];
  v[ 8] = u[15];
  v[ 9] = 0.0;
  v[10] = u[18];
  v[11] = u[16];
  v[12] = u[19];
  v[13] = u[20];
  v[14] = 0.0;
  v[15] = 0.0;
  v[16] = u[23];
  v[17] = u[21];
  v[18] = u[24];
  v[19] = u[25];
  v[20] = u[26];
  v[21] = u[27];
  v[22] = u[28];
  v[23] = 0.0;
  v[24] = 0.0;
  v[25] = 0.0;
  v[26] = u[29];
  v[27] = u[30];
  v[28] = u[31];
  v[29] = 0.0;
  v[30] = 0.0;
  v[31] = u[32];
  v[32] = 0.0;
  
#if 0
  // Verify that there are no non-shiftable entries.
  // for i=1:33; if isnan(shifter(i)); fprintf(1, '  err += std::abs(u[%2d]);\n', i-1); end; end
  double err = 0.0;
  err += std::abs(u[ 0]);
  err += std::abs(u[ 3]);
  err += std::abs(u[ 4]);
  err += std::abs(u[ 5]);
  err += std::abs(u[10]);
  err += std::abs(u[11]);
  err += std::abs(u[12]);
  err += std::abs(u[13]);
  err += std::abs(u[17]);
  err += std::abs(u[22]);
  std::fprintf(stderr, "err = %e\n", err);
#endif
}

// Given a landscape matrix, use row elimination to reach the canonical form
//    [ 1 0 0 0 * * * ]
//    [ 0 1 0 0 * * * ]
//    [ 0 0 1 0 * * * ]
//    [ 0 0 0 1 * * * ]
// If a pivot turns out to be zero, this breaks.
static
bool eliminate(int m, int n, double *A)
{
#define A(i, j) (A[n * (i) + (j)])
  assert(m <= n);
  
  //for (int i=0; i<m; ++i)
  //  for (int j=0; j<n; ++j)
  //    assert(finite(A(i, j)));
  
#if 1
  // Straight-forward left-to-right scan for pivots.
  for (int k=0; k<m; ++k) {
    // find pivot in column below A(k, k).
    int best_i = k;
    for (int i=k+1; i<m; ++i)
      if (std::abs(A(i, k)) > std::abs(A(best_i, k)))
        best_i = i;
     
    // swap rows to put pivot at A(k, k).
    if (best_i != k) {
      for (int j=k; j<n; ++j) {
        double t = A(k, j);
        A(k, j) = A(best_i, j);
        A(best_i, j) = t;
      }
    }
    
    //std::fprintf(stderr, "pivot(%2d, %2d) = %+e\n", k, k, A(k, k));
    if (A(k, k) == 0.0) {
      //  there is not much we can do about this here.
      return false;
    }
    
    // scale row k.
    for (int j=k+1; j<n; ++j)
      A(k, j) /= A(k, k);
    A(k, k) = 1.0;
     
    // reduce other rows.
    for (int i=0; i<m; ++i) {
      if (i == k)
        continue;
      for (int j=k+1; j<n; ++j)
        A(i, j) -= A(i, k) * A(k, j);
      A(i, k) = 0.0;
    }
  }
#endif
  
#if 0
  // Try harder to find a good pivot.
  bool red[18]; assert(m <= 18);
  for (int k=0; k<m; ++k)
    red[k] = false;
  
  while (true) {
    //print(stderr, "A", m, n, A);
    
    // find pivot.
    int best_i = -1;
    int best_j = -1;
    double best_v = 0.0;
    for (int j=0; j<m; ++j) {
      if (red[j])
        continue;
      for (int i=0; i<m; ++i) {
        double v = std::abs(A(i, j));
        if (v > best_v) {
          best_i = i;
          best_j = j;
          best_v = v;
        }
      }
    }
    if (best_v == 0.0)
      // nothing to do.
      break;
    
    // swap rows to put pivot on diagonal.
    int const k = best_j;
    if (best_i != k) {
      for (int j=k; j<n; ++j) {
        double t = A(k, j);
        A(k, j) = A(best_i, j);
        A(best_i, j) = t;
      }
    }
    
    //std::fprintf(stderr, "pivot(%2d, %2d) = %+e\n", k, k, A(k, k));
    if (A(k, k) == 0.0) {
      // fail.
      return false;
    }
    
    // scale row k.
    for (int j=k+1; j<n; ++j)
      A(k, j) /= A(k, k);
    A(k, k) = 1.0;
    
    // reduce other rows.
    for (int i=0; i<m; ++i) {
      if (i == k)
        continue;
      for (int j=k+1; j<n; ++j)
        A(i, j) -= A(i, k) * A(k, j);
      A(i, k) = 0.0;
    }
    
    // mark it as reduced.
    red[k] = true;
  }
#endif
  
  //for (int i=0; i<m; ++i)
  //  for (int j=0; j<n; ++j)
  //    assert(finite(A(i, j)));
  
  return true;
#undef A
}

static
bool solve_linear(double const x[6][3],
                  double const y[6][3],
                  double X[3][3][3])
{
  // form linear system for F.
  double design[6][3][3];
  for (int n=0; n<6; ++n) {
    for (int i=0; i<3; ++i)
      for (int j=0; j<3; ++j)
        design[n][i][j] = x[n][i] * y[n][j];
    normalize(3*3, &design[n][0][0]);
  }
  //print(stderr, "design", 6, 9, &design[0][0][0]);
  
  // column pivoted QR decomposition, landscape case:
  //
  // [* * *|* * *|* * *]   [* * * * * *]  [* * * * * *|* * *]
  // [* * *|* * *|* * *]   [* * * * * *]  [0 * * * * *|* * *]
  // [* * *|* * *|* * *] = [* * * * * *]  [0 0 * * * *|* * *]
  // [* * *|* * *|* * *]   [* * * * * *]  [0 0 0 * * *|* * *]
  // [* * *|* * *|* * *]   [* * * * * *]  [0 0 0 0 * *|* * *]
  // [* * *|* * *|* * *]   [* * * * * *]  [0 0 0 0 0 *|* * *]
  //
  double R[6][9];
  int p[9];
  vf_qr(6, 9, &design[0][0][0], &R[0][0], p);
  //print(stderr, "R", 6, 9, &R[0][0]);
  //std::fprintf(stderr, "p = ["); for (int j=0; j<9; ++j) std::fprintf(stderr, " %d", p[j]); std::fprintf(stderr, " ]\n");
  
  // extract three null-vectors, by back-substitution.
  for (int k=0; k<3; ++k) {
    // solution is a 9-vector.
    double *sol = &X[k][0][0];
    
    // We look for solutions of this form:
    //   [* * *]  0
    //   [* * *]  1
    //   [* * *]  2
    //   [* * *]  3
    //   [* * *]  4
    //   [* * *]  5
    //   [x x x]  6
    //   [y y y]  7
    //   [z z z]  8
    // where the bottom 3 x 3 is prescribed.
    static double const tail[3][3] = {
      //  x     y     z
      { +1.0, -1.0, -1.0 },
      { -1.0, +1.0, -1.0 },
      { -1.0, -1.0, +1.0 }
    };
    // It might seem more natural to ask for an identity
    // matrix at the bottom but the linear system becomes
    // singular in that case.
    sol[p[6]] = tail[k][0];
    sol[p[7]] = tail[k][1];
    sol[p[8]] = tail[k][2];
    
    // back-substitution.
    for (int l=5; l>=0; --l) {
      if (R[l][p[l]] == 0.0)
        return false; // fail
      double acc = 0;
      for (int t=l+1; t<9; ++t)
        acc += R[l][p[t]] * sol[p[t]];
      sol[p[l]] = - acc / R[l][p[l]];
    }
  }
  
  //// verify.
  //for (int k=0; k<3; ++k) {
  //  print(stderr, "X[k]", 3, 3, &X[k][0][0]);
  //  
  //  for (int n=0; n<6; ++n) {
  //    double xn[3] = { x[n][0], x[n][1], x[n][2] }; normalize(3, xn);
  //    double yn[3] = { y[n][0], y[n][1], y[n][2] }; normalize(3, yn);
  //    
  //    double val = 0.0;
  //    for (int i=0; i<3; ++i)
  //      for (int j=0; j<3; ++j)
  //        val += xn[i] * X[k][i][j] * yn[j];
  //    std::fprintf(stderr, "x_%d X_%d y_%d = %+e\n", n, k, n, val);
  //  }
  //}
  return true;
}

static // z = x / y
void complex_div(double *z_real, double *z_imag,
                 double const *x_real, double const *x_imag,
                 double const *y_real, double const *y_imag)
{
  double const yy = (*y_real)*(*y_real) + (*y_imag)*(*y_imag);
  (*z_real) = (+ (*x_real) * (*y_real) + (*x_imag) * (*y_imag)) / yy;
  (*z_imag) = (- (*x_real) * (*y_imag) + (*x_imag) * (*y_real)) / yy;
}

bool vf_solve(double const x[6][3],
              double const y[6][3],
              double X[][3][3],
              double *x_real, double *x_imag,
              double *y_real, double *y_imag,
              double *p_real, double *p_imag)
{
  // Compute solutions to linear constraints.
  if (! solve_linear(x, y, X)) return false;
  
  // Now set F = F[0] + x F[1] + y F[2] and solve for x, y, p.
  // NB: x = l1, y = l2 in notation of CVPR paper.
  
  // Original system of cubics in E (not cubics in l1, l2, p).
  double A[18][33];
  vf_10_33(&X[0][0][0], &A[0][0]);
  
  //print(stderr, "A_10", 10, 33, &A[0][0]);
  
  times_p(A[ 9], A[10]);
  times_p(A[10], A[11]);
  if (! eliminate(12, 33, &A[0][0])) return false;
  
  //print(stderr, "A_12", 12, 33, &A[0][0]);
  
  times_p(A[ 6], A[12]);
  times_p(A[ 7], A[13]);
  times_p(A[ 8], A[14]);
  times_p(A[ 9], A[15]);
  if (! eliminate(16, 33, &A[0][0])) return false;

  //print(stderr, "A_16", 16, 33, &A[0][0]);
  
  times_p(A[ 7], A[16]);
  times_p(A[ 8], A[17]);
  if (! eliminate(18, 33, &A[0][0])) return false;
  
  //print(stderr, "A_18", 18, 33, &A[0][0]);
  
  // The last 15 of the 33 monomials give a basis for the
  // quotient ring R/I. Using the computed GB we can form
  // the matrices of the action of x or y on the quotient
  // ring wrt the monomial basis.
  
#if 0
  // action matrix for x.
  double m_x[15][15];
  // m_x = [ - A([7 8 12 11 14 3 10 15 17], 19:33)
  //         zeros(1,  0) 1 zeros(1, 14)
  //         zeros(1,  2) 1 zeros(1, 12)
  //         zeros(1,  5) 1 zeros(1,  9)
  //         zeros(1,  6) 1 zeros(1,  8)
  //         zeros(1,  8) 1 zeros(1,  6)
  //         zeros(1, 11) 1 zeros(1,  3) ];
  for (int j=0; j<15; ++j) m_x[ 0][j] = - A[ 6][18 + j];
  for (int j=0; j<15; ++j) m_x[ 1][j] = - A[ 7][18 + j];
  for (int j=0; j<15; ++j) m_x[ 2][j] = - A[11][18 + j];
  for (int j=0; j<15; ++j) m_x[ 3][j] = - A[10][18 + j];
  for (int j=0; j<15; ++j) m_x[ 4][j] = - A[13][18 + j];
  for (int j=0; j<15; ++j) m_x[ 5][j] = - A[ 2][18 + j];
  for (int j=0; j<15; ++j) m_x[ 6][j] = - A[ 9][18 + j];
  for (int j=0; j<15; ++j) m_x[ 7][j] = - A[14][18 + j];
  for (int j=0; j<15; ++j) m_x[ 8][j] = - A[16][18 + j];
  for (int j=0; j<15; ++j) m_x[ 9][j] = ((j ==  0) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_x[10][j] = ((j ==  2) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_x[11][j] = ((j ==  5) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_x[12][j] = ((j ==  6) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_x[13][j] = ((j ==  8) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_x[14][j] = ((j == 11) ? 1.0 : 0.0);
  //print(stderr, "m_x", 15, 15, &m_x[0][0]);
#endif
  
#if 1
  // action matrix for y.
  double m_y[15][15];
  // m_y = [ - A([8 9 11 13 18 10 15 16], 19:33)
  //         zeros(1,  0) 1 zeros(1, 14)
  //         zeros(1,  1) 1 zeros(1, 13)
  //         zeros(1,  3) 1 zeros(1, 11)
  //         zeros(1,  6) 1 zeros(1,  8)
  //         zeros(1,  7) 1 zeros(1,  7)
  //         zeros(1,  9) 1 zeros(1,  5)
  //         zeros(1, 12) 1 zeros(1,  2) ];
  for (int j=0; j<15; ++j) m_y[ 0][j] = - A[ 7][18 + j];
  for (int j=0; j<15; ++j) m_y[ 1][j] = - A[ 8][18 + j];
  for (int j=0; j<15; ++j) m_y[ 2][j] = - A[10][18 + j];
  for (int j=0; j<15; ++j) m_y[ 3][j] = - A[12][18 + j];
  for (int j=0; j<15; ++j) m_y[ 4][j] = - A[17][18 + j];
  for (int j=0; j<15; ++j) m_y[ 5][j] = - A[ 9][18 + j];
  for (int j=0; j<15; ++j) m_y[ 6][j] = - A[14][18 + j];
  for (int j=0; j<15; ++j) m_y[ 7][j] = - A[15][18 + j];
  for (int j=0; j<15; ++j) m_y[ 8][j] = ((j ==  0) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_y[ 9][j] = ((j ==  1) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_y[10][j] = ((j ==  3) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_y[11][j] = ((j ==  6) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_y[12][j] = ((j ==  7) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_y[13][j] = ((j ==  9) ? 1.0 : 0.0);
  for (int j=0; j<15; ++j) m_y[14][j] = ((j == 12) ? 1.0 : 0.0);
  //print(stderr, "m_y", 15, 15, &m_y[0][0]);
#endif
  
  // action matrix for p is not easily computed from this GB.
  
  // This is rather more stuff than we actually need.
  double Dreal[15];
  double Dimag[15];
  double Vreal[15][15];
  double Vimag[15][15];
  vf_eig(15, &m_y[0][0], Dreal, Dimag, &Vreal[0][0], &Vimag[0][0]);
  //print(stderr, "Vreal", 15, 15, &Vreal[0][0]);
  
  for (int k=0; k<15; ++k) {
    if (Dimag[k] == 0.0) {
      // real case.
      x_real[k] = Vreal[11][k] / Vreal[14][k];
      x_imag[k] = 0.0;
      
      y_real[k] = Vreal[12][k] / Vreal[14][k];
      y_imag[k] = 0.0;
      
      p_real[k] = Vreal[13][k] / Vreal[14][k];
      p_imag[k] = 0.0;
    }
    
    else {
      // non-real case
      complex_div(x_real+k, x_imag+k, Vreal[11]+k, Vimag[11]+k, Vreal[14]+k, Vimag[14]+k);
      complex_div(y_real+k, y_imag+k, Vreal[12]+k, Vimag[12]+k, Vreal[14]+k, Vimag[14]+k);
      complex_div(p_real+k, p_imag+k, Vreal[13]+k, Vimag[13]+k, Vreal[14]+k, Vimag[14]+k);
    }
  }
  
  return true;
}

void vf_solve(double const x[6][3],
              double const y[6][3],
              double f[15],
              double F[15][3][3])
{
  double X[3][3][3];
  double x_real[15], x_imag[15];
  double y_real[15], y_imag[15];
  double p_real[15], p_imag[15];
  if (! vf_solve(x, y, X, x_real, x_imag, y_real, y_imag, p_real, p_imag)) {
    // it is all bad.
    for (int k=0; k<15; ++k) {
      f[k] = 0.0;
      for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
          F[k][i][j] = 0.0;
    }
    return;
  }
  
  // tolerance of non-realness.
  double const real_tol = 1e-8;
  
  // compose f and F from solutions.
  for (int k=0; k<15; ++k) {
    //std::fprintf(stderr, "  D = (%+e, %+e)\n", Dreal[k], Dimag[k]);
    
    if (((x_imag[k] == 0.0) || (std::abs(x_imag[k]) <= real_tol)) &&
        ((y_imag[k] == 0.0) || (std::abs(y_imag[k]) <= real_tol)) &&
        ((p_imag[k] == 0.0) && (p_real[k] >= 0.0))) {
      // real solution with real focal length.
      double const x = x_real[k];
      double const y = y_real[k];
      double const p = p_real[k];
      //std::fprintf(stderr, "  x, y, p = %+e, %+e, %+e\n", x, y, p);
      
      f[k] = 1.0 / std::sqrt(p);
      for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
          F[k][i][j] = X[0][i][j] + x * X[1][i][j] + y * X[2][i][j];
      normalize(3*3, &F[k][0][0]);
    }
    
    else {
      // complex solution - zero f and F.
      f[k] = 0.0;
      for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
          F[k][i][j] = 0.0;
    }
  }
}

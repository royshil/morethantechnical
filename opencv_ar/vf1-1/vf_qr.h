#ifndef vf_qr_h_
#define vf_qr_h_

// A: m x n matrix.
// R: m x n matrix.
// p: array of size n. column permutation of {0, 1, .., n-1}.
void vf_qr(int m, int n, double const *A, double *R, int p[]);

#endif

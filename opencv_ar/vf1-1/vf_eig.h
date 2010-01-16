#ifndef vf_eig_h_
#define vf_eig_h_

// A: n x n.
// D*: n.
// V*: n x n.
void vf_eig(int n, double const *A, double *Dreal, double *Dimag, double *Vreal, double *Vimag);

#endif

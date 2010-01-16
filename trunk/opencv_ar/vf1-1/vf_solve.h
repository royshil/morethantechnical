#ifndef vf_solve_h_
#define vf_solve_h_

// An implementation of the method described in:
//   H. Stew\'enius, F. Kahl, D. Nist\'er and F. Schaffalitzky.
//   "A Minimal Solution for Relative Pose with Unknown Focal Length".
//   Proceedings of the IEEE Conference on Computer Vision and Pattern
//   Recognition, San Diego, 2005.
//
// Author: F. Schaffalitzky, June 2005.


// Internal calibration of each camera is assumed to be
//   K = [f 0 0]
//       [0 f 0]
//       [0 0 1]
// where f is unknown (and positive). If your principal point is not
// at (0, 0) then you must correct your image points to make it so.
// If your focal length is in the hundreds or thousands, it might be
// helpful to scale your image points first, or it might not. There
// are no guarantees.
//
// Note to HZ fans: the computed Fs satisfy xFy = 0 where x is from
// the "left" image and y is from the "right" image.


// Call this function.
// Invalid solutions are marked with zeros in f.
void vf_solve(double const x[6][3],  // points in left image.
              double const y[6][3],  // points in right image.
              double f[15],          // focal lengths (or zero).
              double F[15][3][3]);   // fundamental matrices.

// Undocumented entry point.
bool vf_solve(double const x[6][3],
              double const y[6][3],
              double F[][3][3],      // three generators for linear F solutions.
              double *x_real, double *x_imag,  // F = F_0 + x F_1 + y * F_2.
              double *y_real, double *y_imag,  //
              double *p_real, double *p_imag); // p = 1 / f^2

#endif

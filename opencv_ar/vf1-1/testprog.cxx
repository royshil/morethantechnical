// Simple test program. It will solve from six points and
// print the focal lengths. If one of the values printed
// is 1.0 then the code probably worked.

#include "vf_solve.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

double draw_uniform()
{
  return std::rand() / double(RAND_MAX);
}

void tester()
{
#if 0
  double x[6][3];
  double y[6][3];
  
  for (int n=0; n<6; ++n) {
    for (int i=0; i<3; ++i) {
      x[n][i] = 2.0 * draw_uniform() - 1.0;
      y[n][i] = 2.0 * draw_uniform() - 1.0;
    }
  }
#else
  double x[6][3] = {
    { -0.45144128256633,  -1.17215627874180,  -0.25412119717348 },
    {  1.99847334720323,   2.18458239825919,   0.27880111219072 },
    { -0.83228971905246,   1.14765560835478,  -0.45457192383734 },
    {  0.38408374014801,   0.40986885926029,   0.53151709742645 },
    {  0.15932829715748,  -0.18554157454543,  -0.19255639213926 },
    { -0.17017171838650,  -0.39024506499604,  -0.19485698032226 }
  };
  double y[6][3] = {
    { -0.64190931495603,  -1.04742079659406,   0.12293723139547 },
    { -0.88302393626983,   1.64921505103104,  -1.13218842697157 },
    { -1.87691139791994,  -1.44148777867567,   0.82770633934862 },
    {  3.11167512733666,  -2.29060009137858,  -0.54236471172232 },
    { -0.29117390717688,   0.02566196382360,  -0.10615607258653 },
    { -1.01308842830859,   2.27503414679349,  -0.62456487517536 }
  };
  if (0) {
    // pick a focal length at random.
    double const f = - std::log(draw_uniform());
    std::fprintf(stderr, "true f = %e\n", f);
    for (int n=0; n<6; ++n) {
      x[n][2] /= f;
      y[n][2] /= f;
    }
  }
#endif
  
  double f[15];
  double F[15][3][3];
  std::fprintf(stderr, "solve...");
  vf_solve(x, y, f, F);
  std::fprintf(stderr, "done.\n");
  
  for (int k=0; k<15; ++k) {
    if (f[k] == 0.0)
      continue;
    
    // print focal length:
    std::fprintf(stderr, "f[%2d] = %e\n", k, f[k]);
    
    // print F-matrix:
    std::fprintf(stderr, "F[%2d][][] = [ ...\n", k);
    for (int i=0; i<3; ++i) {
      for (int j=0; j<3; ++j)
        std::fprintf(stderr, " %+e", F[k][i][j]);
      std::fprintf(stderr, "\n");
    }
    std::fprintf(stderr, "];\n");
  }
  
  //// timing.
  //int const T = 10000;
  //std::fprintf(stderr, "solving %d times...", T);
  //for (int t=0; t<T; ++t)
  //  vf_solve(x, y, f, F);
  //std::fprintf(stderr, "done\n");
}

int main(int, char **)
{
  //while (true) tester();
  tester();
  return 0;
}

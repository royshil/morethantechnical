//      N: non-negative integer.
//      
//   src1: C array of size N x  3 x  3 x  3.
//         [ X000 X001 X002 X010 X011 X012 X020 X021 X022 X100 X101 X102 X110 X111 X112 X120 X121 X122 X200 X201 X202 X210 X211 X212 X220 X221 X222 ]
//      
//   dst1: C array of size N x 10 x 33.
void vf_10_33(int N, double const *src1, double *dst1);
void vf_10_33(/*N=1*/double const *src1, double *dst1);

% Create three camera matrices
P{1} = [-415.59 -573.12 588.74 -96098.28;
         818.72 -342.40 244.05  91817.98;
           0.10    0.70   0.70   1125.40];

P{2} = [-816.61 -633.27  -25.64  111178.57;
        -300.94  351.53  924.35   46418.66;
          -0.55    0.69   -0.47    1129.02];

P{3} = [ 586.30 -384.93 -809.23 -186159.29;
        -872.81  -25.98 -619.80   37008.69;
           0.17    0.94   -0.30     894.35];

  
% Create three image points
x{1} = [-247.39 -138.92]';
x{2} = [138.50 -93.14]';
x{3} = [-86.50 335.34]';

% Find the solution by the QR-method
X = tvt_solve_qr(P,x);
% Find the solution by the standard-method
%X = tvt_solve_std(P,x);
% Find the solution by the SVD-method
%X = tvt_solve_qr(P,x);

% Find the reprojection errors
x1tmp = P{1}*[X; 1];
x1tmp(1:2)/x1tmp(3)-x{1}(1:2)

x2tmp = P{2}*[X; 1];
x2tmp(1:2)/x2tmp(3)-x{2}(1:2)

x3tmp = P{3}*[X; 1];
x3tmp(1:2)/x3tmp(3)-x{3}(1:2)


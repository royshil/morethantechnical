%vgg_X_from_xP_nonlin  Estimation of 3D point from image matches and camera matrices, nonlinear.
%   X = vgg_X_from_xP_lin(x,P,imsize) computes max. likelihood estimate of projective
%   3D point X (column 4-vector) from its projections in K images x (3-by-K matrix)
%   and camera matrices P (K-cell of 3-by-4 matrices). Image sizes imsize (2-by-K matrix)
%   are needed for preconditioning.
%   By minimizing reprojection error, Newton iterations.
%
%   X = vgg_X_from_xP_lin(x,P,imsize,X0) takes initial estimate of X. If X0 is omitted,
%   it is computed by linear algorithm.
%
%   See also vgg_X_from_xP_lin.

% werner@robots.ox.ac.uk, 2003

function X = X_from_uP_nonlin(u,P,imsize,X)

if iscell(P)
  P = cat(3,P{:});
end
K = size(P,3);
if K < 2
  error('Cannot reconstruct 3D from 1 image');
end

if nargin<4
  X = vgg_X_from_xP_lin(u,P,imsize);
end

% precondition
imsize = mean(imsize,2);
f = 4/sum(imsize);
H = [f 0 -f*imsize(1)/2
     0 f -f*imsize(2)/2
     0 0  1             ];
for k = 1:K
  P(:,:,k) = H*P(:,:,k);
end
u = homu(H,u);

% Parametrize X such that X = T*[Y;1]; thus x = P*T*[Y;1] = Q*[Y;1]
[dummy,dummy,T] = svd(X',0);
T = T(:,[2:end 1]);
for k = 1:K
  Q(:,:,k) = P(:,:,k)*T;
end

% Newton
Y = [0;0;0];
eprev = inf;
for n = 1:10
  [e,J] = resid(Y,u,Q);
  if 1-norm(e)/norm(eprev) < 1000*eps
    break
  end
  eprev = e;  
  Y = Y - (J'*J)\(J'*e);
end

X = T*[Y;1];

return

%%%%%%%%%%%%%%%%%%%%%%%%%%

function [e,J] = resid(Y,u,Q)
K = size(Q,3);
e = [];
J = [];
for k = 1:K
  q = Q(:,1:3,k);
  x0 = Q(:,4,k);
  x = q*Y + x0;
  e = [e; x(1:2)/x(3)-u(:,k)];
  J = [J; [x(3)*q(1,:)-x(1)*q(3,:)
           x(3)*q(2,:)-x(2)*q(3,:)]/x(3)^2];
end
return

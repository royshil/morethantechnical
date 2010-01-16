function [X, min_err] = tvt_solve_svd(Porg,x)
% TVT_SOLVE_SVD - Optimal three view triangulation
% 
% X = TVT_SOLVE_SVD(Porg,x)
%
% Input:
%   Porg - cell list with three 3x4 camera matrices
%   x - cell list with the coordinates for the corresponding points.
%
% Output:
%   X - The triangulated 3D-point
%   min_err - the reprojection error
%
% Klas Josephson, Martin Byrod 2008-06-19
%
%@conference{byrod-josephson-etal-iccv-07,
%   author = "Byr{\"o}d, Martin and Josephson, Klas and {\AA}str{\"o}m, Kalle",
%   title = "Improving Numerical Accuracy of Gr{\"o}bner Basis Polynomial Equation Solvers",
%   booktitle ="International Conference on Computer Vision",
%   year = "2007"
%}
%
%

% Normalize camera scale
xx = [Porg{1}(:)
    Porg{2}(:)
    Porg{3}(:)];
m = mean(log10(abs(xx)));
m = 10^m;
for k = 1 : 3
    Porg{k} = Porg{k}/m;
end

%%%%%%%%%%%%%%%%%
% Change coordinate systems
%%%%%%%%%%%%%%%%%

% Start with the image systems
P =cell(1,3);
imH = eye(3)*1e-3;
imH(3,3) = 1;
for ii = 1:3
    imH(1:2,3) = -x{ii}(1:2)*1e-3;
    P{ii} = imH*Porg{ii};
end

% Now to the world coordinate system
% Set up conditions for the elements in Hworld
H_eq = zeros(16,16);
% Go through the cameras
for ii = 1:3
    % Go through the 4 elements
    for jj = 1:4
        col = (4*(jj-1)+1):(4*jj);
        H_eq(4*(ii-1)+jj,col) = P{ii}(end,:);
    end
end

% Add one equation so that the column sum is 1
H_eq(13,1:4) = 1;
H_eq(14,5:8) = 1;
H_eq(15,9:12) = 1;
H_eq(16,13:16) = 1;
sol = [1 0 0 0 0 1 0 0 0 0 1 0 1 1 1 1]';
Hworld = reshape(H_eq\sol,4,4);


% Make the coordinate system change
for ii = 1:3
   P{ii} = P{ii}*Hworld; 
end


%%%%%%%%%%%%%%%%%
% Set up equations
%%%%%%%%%%%%%%%%%
coeff_matrix  = tvt_setup_equations(P);

% Get some numbers that are good to have
nbr_mons = 209;

% Extract monomials that can NOT be multiplied
non_permissible = 1:55;

% Extract monomials that CAN be multiplied
permissible = 56:nbr_mons;
nbr_per = length(permissible);

% Project onto the orthogonal complement to those not allowed
uu = triu(lu(coeff_matrix));

% Do svd on the permissible part
[u,s,v] = svd(uu(permissible(1:104),permissible));

% Set the number of basis elements
n_basis = 50;


% Find the indeces for the basis elsements
basis = nbr_mons - n_basis+1 : nbr_mons;
% and the non-basis
top = 1 : nbr_mons - n_basis;


% Start to create the modulu matrix
T = zeros(nbr_mons - n_basis, n_basis);
% Do the elimination and put it in place
T(non_permissible, :) = uu(non_permissible,non_permissible)\...
    (uu(non_permissible,permissible)*v(:, end - n_basis+1 : end));


% Begin with the modulus matrix
mod_matrix = zeros(n_basis, nbr_mons);
mod_matrix(:,basis) = eye(n_basis);
mod_matrix(:,top) = -T';


% build action matrix from basis to R[x,y,z]
Ahat_x = zeros(nbr_mons,length(permissible));


% The x, y and z multiplications indices
ind_x = [1:44 56:88 100:121 133:136 139 45 89:91 122:127 137 138 140:153 155:158 170 171 159 172:179 186:189 191:195];

% Create the big action matrix
Ahat_x(ind_x, :) = eye(nbr_per);

% change basis for Ahat
ve = eye(nbr_mons);
ve(permissible, permissible) = v;
Ahat_x_t = ve'*Ahat_x*ve(permissible,:);

% generate the "real" action matrix
A_x_t = mod_matrix * Ahat_x_t;
A_x_t = A_x_t(:, basis);


% Calculate the eigen vectors of the transposed action matrix
[vx_t,dx_t] = eig(A_x_t');
ind = imag(diag(dx_t))==0;


%%%%%%%
% go back to the original basis

% define lift mapping (R[x]/I -> R[x])
lift = zeros(nbr_mons,n_basis);
lift(basis, :) = eye(n_basis);
% define the basis-change-matrix in the quotient ring
Vq = mod_matrix * ve' * lift;


% Extract the solutions
sol_x = Vq'*vx_t(:,ind);
sols = sol_x([36 40 41],:)./(ones(3,1)*sol_x(50,:));


%%%%%%%%%%%%%%%%%
% Find the best solution
%%%%%%%%%%%%%%%%%

% Find the minimum reprojection error
min_err = inf;
ind = 0;
for ii = 1:size(sols,2)
    x1 = P{1}*[sols(:,ii); 1];
    x2 = P{2}*[sols(:,ii); 1];
    x3 = P{3}*[sols(:,ii); 1];
    err = sum((x1(1:2)/x1(3)).^2+(x2(1:2)/x2(3)).^2+(x3(1:2)/x3(3)).^2);
    if err<min_err
        min_err=err;
        ind = ii;
    end
end

% Change coordinates back
X = Hworld*[sols(:,ind); 1]; 
X = X(1:3)/X(4);

% Find the minimal reprojection error in the original coordinates
x1 = Porg{1}*[X; 1];
x2 = Porg{2}*[X; 1];
x3 = Porg{3}*[X; 1];
min_err = sum((x1(1:2)/x1(3)-x{1}).^2 + (x2(1:2)/x2(3)-x{2}).^2 + (x3(1:2)/x3(3)-x{3}).^2);


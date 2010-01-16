function [X, min_err] = tvt_solve_qr(Porg,x)
% TVT_SOLVE_QR - Optimal three view triangulation
% 
% X = TVT_SOLVE_QR(Porg,x)
%
% Input:
%   Prog - cell list with three 3x4 camera matrices
%   x    - cell list with the coordinates for the corresponding points.
%
% Output:
%   X - The triangulated 3D-point
%   min_err - the reprojection error
%
% Klas Josephson, Martin Byrod 2008-06-19
%
%
%@conference{byrod-josephson-etal-eccv-08,
%  author        = "Byr{\"o}d, Martin and Josephson, Klas and {\AA}str{\"o}m, Kalle",
%  title         = "A Column-Pivoting Based Strategy for Monomial Ordering in Numerical Gr{\"o}bner Basis Calculations",
%  booktitle     = "10th European Conference on Computer Vision",
%  address       = "Marseille, France",
%  year          = "2008"
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

%%%%%%%%%%%%%%%%%
% Solve the problem
%%%%%%%%%%%%%%%%%
[nbr_eqs, nbr_mons] = size(coeff_matrix);

% Extract monomials that can NOT be multiplied
non_permissible = 1:55;

% Extract monomials that CAN be multiplied
permissible = 56:nbr_mons;
nbr_per = length(permissible);

% Project onto the orthogonal complement to those not allowed
uu = triu(lu(coeff_matrix));

% Do qr on the permissible part
[Q R2 E] = qr(uu(permissible(1:104),permissible), 0);

% Set the number of basis elements
n_basis = sum(~(abs(R2(1)./diag(R2))<1e8))+50;
basis = (nbr_mons-n_basis+1):nbr_mons;
non_basis = 1:(nbr_mons-n_basis);

% Construct the new coefficent matrix
C = [uu(non_permissible,non_permissible) ...
    uu(non_permissible,permissible(E));...
    zeros(nbr_per-50,length(non_permissible)) R2];

% Do the elimination and put it in placae
T= C(non_basis,non_basis)\C(non_basis,basis);

% Create the modolu matrix
mod_matrix = zeros(n_basis, nbr_mons);
mod_matrix(:,basis) = eye(n_basis);

% Complete the modulo matrix
mod_matrix(:,non_basis) = -T';

% The x, y and z multiplications indices
ind_x = [1:44 56:88 100:121 133:136 139 45 89:91 122:127 137 138 140:153 155:158 170 171 159 172:179 186:189 191:195];
ind_y = [2 4 5 7:9 11:14 16:20 22:27 29:35 37:44 46:53 57 59 60 62:64 66:69 71:75 77:82 84:90 92:96 101 103 104 106:108 110:113 115:119 121:126 128 134 136 137 139 143 54 97:99 129:132 161 162 140 141 144:146 148:152 154 164:168 156 158 170 159 172 173 175:178 180:184 187 189 190 192 194 196:199];
ind_z = [3 5 6 8:10 12:15 17:21 23:28 30:36 38:45 47:54 58 60 61 63:65 67:70 72:76 78:83 85:91 93:97 102 104 105 107 108 109 111:114 116:120 122:127 129 135 137 138 140 144 55 98 99 160 130:132 161:163 141 142 145:147 149:153 164:169 157 170:174 176:179 181:185 188 190 191 193 195 197:200];

% Find the indices to create the action matrix 
[slask, sort_order] = sort(E);
sort_order = [non_permissible sort_order+55];
indx = sort_order(ind_x(E(basis-55)));
indy = sort_order(ind_y(E(basis-55)));
indz = sort_order(ind_z(E(basis-55)));

% find the solutions by eigen-decomposition
[v1_t d1] = eig(mod_matrix(:,indx)');

% Find the real solutions
ind = find(imag(diag(d1))==0);
v1_t = v1_t(:,imag(diag(d1))==0);

% Use the fast eigenvalue method
d1 = diag(d1(ind,ind));
d2 = sort(mod_matrix(:,indy)'*v1_t./v1_t);
d3 = sort(mod_matrix(:,indz)'*v1_t./v1_t);

% To use the median
med = floor(n_basis/2);

% construct the solution
sols = [d1'; d2(med,:); d3(med,:)];


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


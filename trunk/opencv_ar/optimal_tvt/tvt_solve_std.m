function [X, min_err] = tvt_solve_std(Porg,x)
% TVT_SOLVE_STD - Optimal three view triangulation
% 
% X = TVT_SOLVE_STD(Porg,x)
%
% Input:
%   Porg - cell list with three 3x4 camera matrices
%   x    - cell list with the coordinates for the corresponding points.
%
% Output:
%   X - The triangulated 3D-point
%   min_err - the reprojection error
%
% Klas Josephson, Martin Byrod 2008-06-18
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
[nbr_eqs, nbr_monoms] = size(coeff_matrix);

% Make an initial LU step
uu = triu(lu(coeff_matrix));

% Build the groebner basis
T = uu(1:159,1:159)\uu(1:159,160:end);
%T = coeff_matrix(:,1:159)\coeff_matrix(:,160:end);

% Start to create the action matrix

% Create the modolu matrix
mod_matrix = zeros(50,nbr_monoms);
mod_matrix(:,160:end) = eye(50);

% Complete the modulo matrix
mod_matrix(:,1:159) = -T(1:159,:)';

% The x multiplications indeces
ind_x = [45 89:91 122:127 137 138 140:153 155:158 170 171 159 172:179 186:189 191:195]';

% Calculate the eigenvectors of the transposed action matrix
[vx,dx] = eig(mod_matrix(:,ind_x)');

% Find the real solutions
ind = find(imag(diag(dx))==0);

% Extract the solutions
sols = vx([36 40 41],ind)./(ones(3,1)*vx(50,ind));

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


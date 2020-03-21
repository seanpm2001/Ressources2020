function [c, ceq] = constraints(x)
%UNTITLED9 Summary of this function goes here
%   Detailed explanation goes here
c = [];

% load parameters from workspace
m = evalin('base','m_t');
xf = evalin('base','xf');
xr = evalin('base','xr');
R_turn = evalin('base','R_turn');

% x =  [a_y, SA_f, SA_r]

% calcul des charges
[Y_fi, Y_fe, Y_ri, Y_re] = reactions2(x, R_turn);

ceq(1) = Y_fi +Y_fe + Y_ri + Y_re - m*x(1);
ceq(2) = xr*(Y_ri+Y_re) - xf*(Y_fi+Y_fe);
end

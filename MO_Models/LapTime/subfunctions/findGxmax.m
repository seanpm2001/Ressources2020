function Gx = findGxmax(Gy,V,GGV)
%aproximation de Gx max connaissant Gy et V en utilisant une ellipse comme approximation du cercle de traction de la voiture
% elips : Gx(V)^2/Gx_max(V)^2 + Gy(V)^2/Gy_max(V)^2 = 1

Gxmax_GGV = GGV(:,1);
Gy_GGV = GGV(:,3);
V_GGV = GGV(:,4);

A = interp1(V_GGV,Gxmax_GGV,V,'linear','extrap');
B = interp1(V_GGV,Gy_GGV,V,'linear','extrap');
if Gy>B 
    Gx =0;
else
    Gx = A*sqrt(1-Gy^2/B^2); 
end

end
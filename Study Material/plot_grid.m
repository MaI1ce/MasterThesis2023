Xlim = 24;
Ylim = 24;
d = 2;
n = (Xlim-d)./d;
x = [d:d:Xlim-d];
y = [d:d:Ylim-d];
r = 0.15;
c = 'k';
figure(2)
shft = d;

X_zero = d*5;
Y_zero = d*6;

for i = 1:n
    for j = 1:n
        shft = mod(shft, d*2);
        hold on
        circleplot(x(i)+shft,y(j),r,c,1);
        shft = shft + d;
    end
end

%hold on
%p = polyshape([3*d 4*d 6*d 5*d],[2*d 3*d 3*d 2*d]); rhomb
%p = polyshape([3*d 3*d 5*d 5*d],[2*d 4*d 4*d 2*d]); 
%plot(p)


hold on
q1 = quiver(X_zero,Y_zero,d,d,1,'linewidth',1.5);
q1.MaxHeadSize = 2;

hold on
q2 = quiver(X_zero,Y_zero,d*2,0,1,'linewidth',1.5);
q2.MaxHeadSize = 2;

%hold on
%circleplot(X_zero,Y_zero,d*sqrt(2),'',0);

hold on
circleplot(X_zero,Y_zero,2*d,'',0);

xlim([0 Xlim]);
ylim([0 Ylim])
hold off
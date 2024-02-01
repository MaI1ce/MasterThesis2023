function circle = circleplot(x,y,r,c,f)
th = 0:pi/50:2*pi;
x_circle = r * cos(th) + x;
y_circle = r * sin(th) + y;
circle = plot(x_circle, y_circle);
if f == 1
    hold on
    fill(x_circle, y_circle, c);
end
%plot(x, y, 'p', 'MarkerSize',15, 'MarkerFaceColor','r');
axis equal
hold off
end
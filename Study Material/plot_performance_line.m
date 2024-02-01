x = 1:1:7;
y = [31838366, 41377421, 48216108 ,57800064 ,65932164, 77879201, 85853106]; %std

figure
hold on
loglog(x,y)

y1 = [51725075, 97952389, 147059891, 195574627, 241858444, 283140536, 340633432]; %new
hold on
loglog(x,y1)

xlabel('Number of rounds to sign') 
ylabel('CPU cycles') 
title('CPU cycles for signature')
legend('std-Dilithium','new-Dilithium')
grid on
hold off


y2 = [10934120, 15814613, 21889042, 26751347, 31390636, 37832925, 41336197]; %std-aes
y3 = [14171290, 25867741, 35453180, 42922817, 56687722, 66285079, 73740427]; %new-aes
figure
hold on
loglog(x,y)
hold on
loglog(x,y2)
hold on
loglog(x,y3)
xlabel('Number of rounds to sign') 
ylabel('CPU cycles') 
title('CPU cycles for signature')
legend('std-Dilithium','std-aes-Dilithium', 'new-aes-Dilithium')
grid on


y4 = [27761161, 33264760, 37422028, 41987024, 45893814, 49729704, 55045768]; %std-const-y
figure
hold on
loglog(x,y)
hold on
loglog(x,y4)
xlabel('Number of rounds to sign') 
ylabel('CPU cycles') 
title('CPU cycles for signature')
legend('std-Dilithium','std-const-y-Dilithium')
grid on
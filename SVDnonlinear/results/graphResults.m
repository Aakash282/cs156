X = load('f1_pred.dta');
Y = load('f1_rat.dta');
Y = -1 .* Y;
figure;
plot(X, Y, 'd');
p = polyfit(X, Y, 1)

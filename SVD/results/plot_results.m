Y = [0.9829 0.95921 0.94726 0.94209 0.93846 0.9358 0.92344 0.91508 0.91206 0.91019 0.90717];
X = [0 1 2 3 4 5 10 20 29 39 100];
plot(X, Y);
xlabel('Features')
ylabel('RMSE')
title('SVD RMSE vs number of features')
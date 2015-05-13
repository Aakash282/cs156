# week1_4.py
# author: Aakash Indurkhya
# date: April 16, 2014

import numpy as np
import matplotlib.pyplot as plt


MOVIES = 17770
USERS = 458293

METRIC = USERS

f = open('um/uavg.dta', 'r')


n = [0.0 for a in range(METRIC)]
avg = [0.0 for a in range(METRIC)]

for a in range(METRIC):
	# tokenize the line
	l = f.next().strip().split()
	n[a] = float(l[0])
	avg[a] = float(l[1])

f.close()


plt.scatter(avg, n)
plt.xlabel('Number of Ratings by User (n)')           # and here ?
plt.ylabel('Average User Rating')          # and here ?
plt.title('Average rating vs n')   # and here ?
plt.show()
plt.close()
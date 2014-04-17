# week1_5.py
# author: Aakash Indurkhya
# date: 4/17/14
# summary: Here we get the stdevev by movie in O(n). Results are stored
# in um/ustdev.dta
from numpy import *

# setup
stdev = open('um/ustdev.dta', 'w')
size = open('um/uavg.dta', 'r')
idx = open('um/all.idx', 'r')
current = 1 # always starts with movie 1
n = int(size.next().strip().split()[1])
ratings = [0.0 for a in range(n)]
new_user = False
counter = 0
# iterate through the data file one line at a time
with open('um/all.dta', 'r') as f:
    for s in f:
    	# tokenize the raw data line
        l = s.strip().split()


        # only use valid indices
        index = int(idx.next().strip())
        if index >= 4:
        	continue

        # are we considering a new user
        new_user = int(l[0]) != current

        # save results in file
        if (new_user):
            result = '{0}\t{1}\n'.format(round(std(ratings), 5), n)
            stdev.write(result)
            n = int(size.next().strip().split()[1])
            ratings = [0.0 for a in range(n)]
            current = int(l[0])
            counter = 0

        # ratings

        ratings[counter] = float(l[3])
        counter += 1

        # for progress updates on terminal
        if ((int(l[0]) % 10000 == 0) and new_user):
        	print l[0]
        	new_user = False

    # save results for last movie
    result = '{0}\t{1}\n'.format(round(std(ratings), 5), n)
    stdev.write(result)

f.close()
size.close()
stdev.close()

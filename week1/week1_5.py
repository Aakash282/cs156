# week1_5.py
# author: Aakash Indurkhya
# date: 4/17/14
# summary: Here we get the stdevev by movie in O(n). Results are stored
# in mu/mstdev.dta
from numpy import *

# setup
stdev = open('mu/mstdev.dta', 'w')
size = open('mu/mavg.dta', 'r')
idx = open('mu/all.idx', 'r')
current = 1 # always starts with movie 1
n = int(size.next().strip().split()[1])
ratings = [0.0 for a in range(n)]
new_movie = False
counter = 0
# iterate through the data file one line at a time
with open('mu/all.dta', 'r') as f:
    for s in f:
    	# tokenize the raw data line
        l = s.strip().split()


        # only use valid indices
        index = int(idx.next().strip())
        if index >= 4:
        	continue

        # are we considering a new movie
        new_movie = int(l[1]) != current

        # save results in file
        if (new_movie):
            result = '{0}\t{1}\n'.format(round(std(ratings), 5), n)
            stdev.write(result)
            n = int(size.next().strip().split()[1])
            ratings = [0.0 for a in range(n)]
            current = int(l[1])
            counter = 0

        # ratings

        ratings[counter] = float(l[3])
        counter += 1

        # for progress updates on terminal
        if ((int(l[1]) % 100 == 0) and new_movie):
        	print l[1]
        	new_movie = False

    # save results for last movie
    result = '{0}\t{1}\n'.format(round(std(ratings), 5), n)
    stdev.write(result)

f.close()
size.close()
stdev.close()

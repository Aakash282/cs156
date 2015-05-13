# week1_2.py
# author: Aakash Indurkhya
# date: 4/15/14
# summary: Here we get the avg rating by movie in O(n). Results are stored
# in mu/mavg.dta
from numpy import *

# setup
avg = open('mu/mavg.dta', 'w')
idx = open('mu/all.idx', 'r')
current = 1 # always starts with movie 1
total, n = 0, 0
new_movie = False

# iterate through the data file one line at a time
with open('mu/all.dta', 'r') as f:
    for s in f:
    	# tokenize
        l = s.strip().split()

        # only use valid indices
        index = int(idx.next().strip())
        if index >= 4:
        	continue

        new_movie = int(l[1]) != current

        # save results in file
        if (new_movie):
        	result = '{0}\t{1}\n'.format(round(float(total) / float(n), 5), n)
        	avg.write(result)
        	total = 0
        	n = 0
        	current = int(l[1])

        # collect total
        total += int(l[3])
        n += 1

        # for progress updates on terminal
        if ((int(l[1]) % 100 == 0) and new_movie):
        	print l[1]
        	new_movie = False

    # save results for last movie
    result = '{0}\t{1}\n'.format(round(float(total) / float(n), 5), n)
    avg.write(result)

f.close()
avg.close()

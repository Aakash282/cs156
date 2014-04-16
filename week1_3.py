# week1_3.py
# author: Aakash Indurkhya
# date: 4/15/14
# summary: Here we get the avg rating by user in O(n). Results are stored
# in mu/mavg.dta
from numpy import *

# setup
avg = open('um/uavg.dta', 'w')
idx = open('um/all.idx', 'r')
current = 1 # always starts with movie 1
total, n = 0, 0
new_user = False

# iterate through the data file one line at a time
with open('um/all.dta', 'r') as f:
    for s in f:
    	# tokenize
        l = s.strip().split()

        # only use valid indices
        index = int(idx.next().strip())
        if index >= 4:
        	continue

        new_user = int(l[0]) != current

        # save results in file
        if (new_user):
        	result = '{0}\t{1}\n'.format(round(float(total) / float(n), 5), n)
        	avg.write(result)
        	total = 0
        	n = 0
        	current = int(l[0])

        # collect total
        total += int(l[3])
        n += 1

        # for progress updates on terminal
        if ((int(l[0]) % 10000 == 0) and new_user):
        	print l[0]
        	new_user = False

    # save results for final user 
    result = '{0}\t{1}\n'.format(round(float(total) / float(n), 5), n)
    avg.write(result)

f.close()
avg.close()

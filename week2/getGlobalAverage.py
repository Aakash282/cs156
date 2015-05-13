# Get global average
from numpy import *

# setup
idx = open('../../netflix/mu/all.idx', 'r')
current = 1 # always starts with movie 1
total, n = 0, 0
new_movie = False

# iterate through the data file one line at a time
with open('../../netflix/mu/all.dta', 'r') as f:
    for s in f:
        # only use valid indices
        index = int(idx.next().strip())
        if index > 4:
        	continue

        # tokenize
        l = s.strip().split()

        # collect total
        total += int(l[3])
        n += 1

        # for progress updates on terminal
        if ((n % 1000000 == 0)):
        	print n
        	new_movie = False

f.close()
print n # 99666408
print total # 359747514
print total / n # 3.609516

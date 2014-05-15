# Get baseline rating for training data 
# using average rating + average Offset

from numpy import *
import linecache
# setup
idx = open('../../netflix/mu/all.idx', 'r')
mu_baseline = open('../stats/mu_baseline.dta', 'w')
temp = 0;
movie_avg = -1.0
# iterate through the data file one line at a time
with open('../../netflix/mu/all.dta', 'r') as f:
    for s in f:
        # only use valid indices
        index = int(idx.next().strip())
        if index > 4:
            continue

        # tokenize
        l = s.strip().split()

        user = int(l[0])
        movie = int(l[1])
        
        # Only get new movie avg when switching to new movie
        if (temp != movie):
        	temp = movie
        	if (movie % 1000 == 0):
        		print movie
        	movie_avg = float(linecache.getline('../stats/better_mavg2.dta', movie).strip().split()[0])
        user_offset = float(linecache.getline('../stats/user_offset.dta', user).strip().split()[0])

        estimate = movie_avg + user_offset
        mu_baseline.write('{0}\n'.format(round(estimate, 6)))

idx.close()
f.close()
mu_baseline.close()

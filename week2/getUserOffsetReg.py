# For each user and movie they rated, get avg offset 
# from mean of movie using better_mavg2.dta

from numpy import *
import linecache
# setup
avg = open('../stats/user_offset.dta', 'w')
idx = open('../../netflix/um/all.idx', 'r')
current = 0 # always starts with movie 1
total, n = 0, 0
temp = 1
# iterate through the data file one line at a time
with open('../../netflix/um/all.dta', 'r') as f:
    for s in f:
        # only use valid indices
        index = int(idx.next().strip())
        if index > 4:
            continue

        # tokenize
        l = s.strip().split()

        current = int(l[0])
        movie = int(l[1])
        u_rating = int(l[3])
        # save results in file
        if (temp != current):
            result = '{0}\t{1}\t{2}\n'.format(round((total)/(25.0+n), 6), total, n)
            avg.write(result)
            total = 0
            n = 0
            temp = current
            if (current % 1000 == 0):
                print current


        # collect total
        # Get movie average
        movie_r = float(linecache.getline('../stats/better_mavg2.dta', movie).strip().split()[0])
        total += u_rating - movie_r
        n += 1


    # save results for last movie
    result = '{0}\t{1}\t{2}\n'.format(round((total)/(25.0+n), 6), total, n)
    avg.write(result)

idx.close()
f.close()
avg.close()

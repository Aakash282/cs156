# get better mean
from numpy import *

# setup
avg = open('../stats/better_mavg2.dta', 'w')
idx = open('../../netflix/mu/all.idx', 'r')
current = 0 # always starts with movie 1
total, n = 0, 0
temp = 1
GLOBAL_AVG = 3.609516
# iterate through the data file one line at a time
with open('../../netflix/mu/all.dta', 'r') as f:
    for s in f:
        # only use valid indices
        index = int(idx.next().strip())
        if index > 4:
            continue

        # tokenize
        l = s.strip().split()

        current = int(l[1])

        # save results in file
        if (temp != current):
            result = '{0}\t{1}\t{2}\n'.format(round((GLOBAL_AVG*25.0+total)/(25.0+n), 6), total, n)
            avg.write(result)
            total = 0
            n = 0
            temp = current
            if (current % 1000 == 0):
                print current


        # collect total
        total += int(l[3])
        n += 1


    # save results for last movie
    result = '{0}\t{1}\t{2}\n'.format(round((GLOBAL_AVG*25.0+total)/(25.0+n), 6), total, n)
    avg.write(result)

idx.close()
f.close()
avg.close()

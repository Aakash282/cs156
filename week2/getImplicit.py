# get number of movied rated by each user
from numpy import *

# setup
avg = open('../stats/user_implicit_2.dta', 'w')
current = 0 # always starts with movie 1
total = 0
temp = 1
# iterate through the data file one line at a time
with open('../../netflix/um/all.dta', 'r') as f:
    for s in f:

        # tokenize
        l = s.strip().split()

        current = int(l[0])

        # save results in file
        if (temp != current):
            result = '{0}\n'.format(total)
            avg.write(result)
            total = 0
            temp = current
            if (current % 1000 == 0):
                print current


        # collect total
        total += 1


    # save results for last movie
    result = '{0}\n'.format(total)
    avg.write(result)

f.close()
avg.close()

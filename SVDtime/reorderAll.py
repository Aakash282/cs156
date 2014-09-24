from numpy import *
import linecache
# setup
new = open('../../netflix/um/all_date.dta', 'a')
current = 0 # always starts with movie 1
total, n = 0, 0
temp = 1
# iterate through the data file one line at a time
with open('../../netflix/um/all.dta', 'r') as f:
    for s in f:
        l = s.strip().split()

        user = int(l[0])
        movie = int(l[1])
        date = int(l[2])
        rating = int(l[3])
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
# get number of movied rated by each user
from numpy import *
import math
# setup
avg = open('../stats/user_implicit_2.dta', 'w')
current = 0 # always starts with movie 1
total = 0
temp = 1
# iterate through the data file one line at a time
with open('../stats/user_implicit.dta', 'r') as f:
    for s in f:

        # tokenize
        l = s.strip().split()

        current = int(l[0])
        result = '{0}\t{1}\n'.format(current, round(1 / math.sqrt(float(current)), 6))
        avg.write(result)

f.close()
avg.close()

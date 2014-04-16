# Test blending of avg user rating and avg movie rating on probe set
# probe set is of size 1374739
import numpy as np
PROBE_SIZE = 1374739
# Get probe set
count = 0
idx = open('../netflix/um/all.idx', 'r')
probe = np.zeros((PROBE_SIZE, 4))
with open('../netflix/um/all.dta', 'r') as f:
    for s in f:
        index = int(idx.next().strip())
        if index != 4:
        	continue	
        probe[count] = np.fromstring(s.strip(), dtype=int, sep=' ')
        count += 1

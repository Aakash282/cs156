# Get probe set and save it in stats/probe.dta
PROBE_SIZE = 1374739
probe = open('../stats/probe.dta', 'w')
idx = open('../../netflix/um/all.idx', 'r')

with open('../../netflix/um/all.dta', 'r') as f:
	for s in f:
		index = int(idx.next().strip())
		if index != 4:
			continue
		probe.writelines(s)
f.close()
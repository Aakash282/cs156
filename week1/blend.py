# Test blending of avg user rating and avg movie rating on probe set
# probe set is of size 1374739
import numpy as np
from numpy import matrix
from numpy import linalg
import linecache
PROBE_SIZE = 1374739
QUAL_SIZE = 2749898
TEST_SIZE = QUAL_SIZE
# Get probe set
def load_probe():
	probe = np.genfromtxt('../stats/probe.dta', dtype = int, skip_header = PROBE_SIZE-TEST_SIZE)
	y = (probe[:, -1])
	x = (probe[:, 0:2])
	del probe
	return (x, y)

# X will be either a list of users or list of movies
# Returns a list of the same length with the corresponding
# averages
def get_avgs(x, file):
	avg = np.zeros([1, TEST_SIZE])
	# Initialize
	prev = x[0]
	single_avg = float(linecache.getline(file, prev).strip().split()[0])
	count = 0
	for i in np.nditer(x):
		if i != prev:
			single_avg = float(linecache.getline(file, i).strip().split()[0])
			prev = i
		avg[0][count] = single_avg
		count += 1

	return avg

def find_weights(x, y):
	return (((x*x.T).I) * x) * y.T
	# return 1
	#return (((x.T*x).I) * x.T) *y.T

def train():
	# data: ([user movie], rating)
	x, y = load_probe()
	# matricize data
	user_avg = get_avgs(x[:,0], '../stats/uavg.dta')
	movie_avg = get_avgs(x[:,1], '../stats/mavg.dta')

	x_data = np.matrix([user_avg[0], movie_avg[0], np.ones(TEST_SIZE)])
	y_data = np.matrix(y)

	weights = find_weights(x_data, y_data)
	print weights
	# Weights are 0.77818, 0.963438, -2.682

def get_expected():
	data = np.genfromtxt('../../netflix/um/qual.dta', dtype = int, skip_header=QUAL_SIZE-TEST_SIZE)
	user_avg = get_avgs(data[:,0], '../stats/uavg.dta')
	movie_avg = get_avgs(data[:,1], '../stats/mavg.dta')
	x_data = np.matrix([user_avg[0], movie_avg[0], np.ones(TEST_SIZE)])
	weights = np.matrix([0.77818, 0.963438, -2.682])
	return weights * x_data


# Output ratings to file
def output_ratings(file, ratings):
	output = open(file, 'w')
	for r in np.nditer(ratings):
		output.writelines(str(r) + '\n')
	output.close()

def main():
	#train()
	output_ratings('blend_1.dta', get_expected())

main()

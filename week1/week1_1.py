# Author: Aakash Indurkhya
# Email: aindurkh@caltech.edu
# Date: April 13, 2014
# Summary: Loading in data and taking some averages. Basically just python 
#	refresher and playing with the data. 
# 
from numpy import *

ALL_DATA = 458293
DATA_SIZE = 400000



# open the data files
u = open('../netflix/mu/all.dta', 'r')
idx = open('../netflix/mu/all.idx', 'r')

# initialize the data matrix
D = zeros((0, 4))
# code for appending data
# vstack([D, newrowarray)])

# hashmap for computing avg. 
avg_u = dict()
avg_m = dict()

quiz = []

# iterate through the data
for i in range(DATA_SIZE):
	# tokenize the current line and add the row to the data matrix
	s = u.next().strip()
	index = idx.next().strip()
	if int(index) >= 4:
		if int(index) == 5:
			quiz.append(s.split()[1])

	l = s.split()
	D = vstack([D, l])

	# sort data by user. 
	if l[0] in avg_u:
		# append the new rating to the existing array at this slot
		avg_u[l[0]].append(float(l[3]))
	else: 
		avg_u[l[0]] = [float(l[3])]

	# sort data by movie
	if l[1] in avg_m:
		# append the new rating to the existing array at this slot
		avg_m[l[1]].append(float(l[3]))
	else:
		avg_m[l[1]] = [float(l[3])]


# closing data file. 
u.close()

f = open('blank1.dta', 'w')
for a in quiz:
	s = '{0}'.format(round(mean(avg_m[a]), 3)) + '\n'
	f.write(s)
f.close()
# the rest of the code is just for making sure everything above works

# print some sample data 
for k in avg_m.keys()[:DATA_SIZE]:
	print 'movie: {0}, \taverage: {1}, \tratings: {2}'.format(k, mean(avg_m[k]), len(avg_m[k])) 


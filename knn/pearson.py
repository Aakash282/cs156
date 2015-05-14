# pearson.py
# author: Aakash Indurkhya
# date: 5/6/14
# summary: Here we calculate the Pearson similarity between all users. 


from numpy import *

USERS = 223137

# setup
pearson = open('pearson.dta', 'w')
e = open('um/uavg.dta', 'r')
uavg = e.readlines()
e.close()
f = open('um/all.dta', 'r')
data = f.readlines()
idx = open('um/all.idx', 'r')
new_user = False
i, j, a, u, n = 0, 0, 1, 2, int(uavg[0].strip().split()[1])
movies = [(0, 0) for x in range(n)]
counter = 0

# we need Pearson Correlation between all users (n^2)
while (a < USERS):
	# collect data on this user
	while(not new_user):
		# tokenize the raw data
		# l = linecache.getline('um/all.dta', i).strip().split()
		l = data[i].strip().split()

		# only use valid indices
		index = int(idx.next().strip())
		if index >= 4:
			i += 1
			continue

		# are we considering a new user
		new_user = int(l[0]) != a


		# are we considering a new user
		if (new_user):
			break

		# what movies has this user watched
		# store the movie, rating tuple
		movies[counter] = (int(l[1]), int(l[3]))
		counter += 1
		i += 1

	# for progress updates on terminal
	# if (new_user):
	# 	print a

	j = i

	# now compare this user to all of the other users
	while (u < USERS):
		# this array holds data on the movies the users have in common
		common = []

		print "{0} {1}".format(a, u)

		new_user = False
		# collect data on another user
		while(not new_user):
			# tokenize the daw data
			# l = linecache.getline('um/all.dta', j).strip().split()
			l = data[j].strip().split()

			# only use valid indices
			index = int(idx.next().strip())
			if index >= 4:
				j += 1
				continue

			# are we considering a new user
			new_user = int(l[0]) != u

			# are we considering a new user
			if (new_user):
				a_avg = float(uavg[a-1].strip().split()[0])
				u_avg = float(uavg[u-1].strip().split()[0])
				common = list(set(common))

				# calculate the numerator
				# print common[0]
				num = sum([((common[z][0] - a_avg) * (common[z][1] - u_avg)) for z in range(len(common))])

				# calculate the denominator
				den1 = [(common[z][0] - a_avg) for z in range(len(common))]
				den2 = [(common[z][1] - u_avg) for z in range(len(common))]
				den = sum(power(den1, 2))
				den *= sum(power(den2, 2))

				# calculate the result and store it in the file
				if (num == 0.0 and den == 0.0):
					pearson.write("{0} {1} NA, 0\n".format(a, u, round(result, 5)))
				else:
					result = num / sqrt(den)
					pearson.write("{0} {1} {2} {3}\n".format(a, u, round(result, 5), len(common)))
					# print result
				break

			# if both users have rated the movie, we add it to the list
			for q in range(len(movies)):
				if (int(l[1]) == movies[q][0]):
					common.append((movies[q][1], int(l[3]), int(l[1])))
					break

		# print "finished one user to user comparison"
		u += 1



	# increment our iterators
	a += 1
	u = a + 1
	counter = 0
	n = int(uavg[a-1].strip().split()[1])
	movies = [(0, 0) for x in range(n)]


uavg.close()
idx.close()
pearson.close()


f = open('blank1.dta', 'w')
for a in range(2749899):
	s = '{0}'.format(round(3, 3)) + '\n'
	f.write(s)
f.close()
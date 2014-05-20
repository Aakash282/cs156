# pearson.pyx
# author: Aakash Indurkhya
# date: 5/7/14
# summary: We do the same thing that we did in pearson.py, just faster. 

from numpy import *

def test():
	cdef float x = 5.5345
	print("Hello World %f" % (x))
#!/usr/bin/python

from math import pow

basefreq = 11025
notestep = pow(2.0, 1.0/12.0)

for note in range(0, 24):
  	frequency = basefreq * pow(notestep, note)
	print str(int(frequency))+",",
print

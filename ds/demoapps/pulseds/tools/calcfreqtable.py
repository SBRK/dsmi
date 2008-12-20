#!/usr/bin/python

for note in range(48,176):
	frequency = 16.351597831 * pow(2.0, float(note) / float(12))
	print str(int(frequency))+",",
print

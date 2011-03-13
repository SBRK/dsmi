#!/usr/bin/python

import sys, os
from socket import *

# Send keepalive

host = "localhost"
port_out = 9000
port_in = 9001
buf = 1024
addr = (host, port_out)

UDPSock = socket(AF_INET,SOCK_DGRAM)

UDPSock.sendto(chr(0)+chr(0)+chr(0), addr)
UDPSock.close()

# Recv messages

addr = (host, port_in)
UDPSock = socket(AF_INET,SOCK_DGRAM)
UDPSock.bind(addr)

while True:
	data,addr = UDPSock.recvfrom(buf)
	print "Got msg ",
	for i in range(len(data)):
		print "0x%x" % ord(data[i]),
	print

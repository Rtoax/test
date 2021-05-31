#!/bin/bash

# 
tcpdump -i eth0 -w ./tmp.pcap.out
tcpdump -nr ./tmp.pcap.out

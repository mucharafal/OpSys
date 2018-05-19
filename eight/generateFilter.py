#!/bin/python3
import sys
import random

def generateFilter(c):
	print(c)
	list = []
	sum = 0
	for i in range(c * c):
		num = random.randint(0, 100)
		sum += num
		list = list + [num]

	for i in range(c * c):
		print(list[i] / sum)

def generateFile(x, y):
	print("P2")
	print(x, y)
	print(255)
	m = x * y
	for i in range(m):
		print(random.randint(0, 255))



def main():
	mode = sys.argv[1]
	if mode == "f":
		generateFilter(int(sys.argv[2]))
	else:
		generateFile(int(sys.argv[2]), int(sys.argv[3]))
	

if __name__ == "__main__":
	main()
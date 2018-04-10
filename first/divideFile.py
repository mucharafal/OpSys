import sys

f = open(sys.argv[1], "r")
i = 0
wr = open(sys.argv[2] + str(i), "w")
for line in f:
	if "Porówanie" in line:
		if i != 0:
			wr = open(sys.argv[2] + str(i), "w")
		i += 1
		wr.write(line)
	else:
		wr.write(line)
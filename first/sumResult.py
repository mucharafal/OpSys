import sys


f = open(sys.argv[1], "r")

realTime = 0
totalRealTime = 0

systemTime = 0
totalSystemTime = 0

userTime = 0
totalUserTime = 0

for line in f:
	if "Real" in line:
		line = line.replace("Real Time: ", "")
		line = line.replace(", User Time", "")
		line = line.replace(", System Time", "")
		line = line.replace("\n", "")
		line = line.split(" ")
		realTime = realTime + int(line[0])
		userTime = userTime + int(line[1])
		systemTime = systemTime + int(line[2])
		totalRealTime += int(line[0])
		totalUserTime += int(line[1])
		totalSystemTime += int(line[2])
	elif ":" in line:
		print(line)
		print("Real Time: " + str(realTime))
		print("User Time: " + str(userTime))
		print("System Time " + str(systemTime))
		realTime = 0
		userTime = 0
		systemTime = 0

print("Real Time: " + str(realTime))
print("User Time: " + str(userTime))
print("System Time " + str(systemTime))
print(sys.argv[1])
print("Real Time: " + str(totalRealTime))
print("User Time: " + str(totalUserTime))
print("System Time " + str(totalSystemTime))
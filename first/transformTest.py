f = open("./testSrc", "r")
wr = open("./target", "w")
for line in f:
	if "$" in line:
		wr.write("echo \"" + line + "\"\n" + line)
	else:
		wr.write("echo \"" + line + "\"\n")
f.close()
wr.close()
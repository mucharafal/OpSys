compile: process.o
	gcc process.o -o process -lm -pthread
	
compileO2: process.o
	gcc process.o -O2 -o process -lm -pthread

debug:
	gcc -g process.c -lm -pthread

testThreads:
	./process file.txt filter.txt 1 result.txt >> results.txt
	./process file.txt filter.txt 2 result.txt >> results.txt
	./process file.txt filter.txt 4 result.txt >> results.txt
	./process file.txt filter.txt 8 result.txt >> results.txt

testDiffrentFilters:
	./generateFilter.py m 800 600 > file.txt
	echo "c = 3" >> results.txt
	./generateFilter.py f 3 > filter.txt
	make testThreads
	echo "c = 11" >> results.txt
	./generateFilter.py f 11 > filter.txt
	make testThreads
	echo "c = 21" >> results.txt
	./generateFilter.py f 21 > filter.txt
	make testThreads
	echo "c = 55" >> results.txt
	./generateFilter.py f 55 > filter.txt
	make testThreads

testOptimisation:
	echo "Normal optimisation" >> results.txt
	make compile
	make testDiffrentFilters
	echo "O2 optimisation" >> results.txt
	make compileO2
	make testDiffrentFilters
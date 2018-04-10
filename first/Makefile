all: staticMain

kodOptymalizacji = 0

optymalizacja = -O$(kodOptymalizacji)
wynik = result3bO$(kodOptymalizacji).txt

staticMain: library.o
	gcc $(optymalizacja) main.c library.o  -o staticMain

library.o: ./libraries/library.c
	gcc $(optymalizacja) -c ./libraries/library.c

clean:
	rm *o 

sharedMain: liblibrary.a
	gcc $(optymalizacja) main.c -l library -L. -o sharedMain

liblibrary.a: sharedLibrary.o
	ar rcs liblibrary.a sharedLibrary.o

sharedLibrary.o:
	gcc -fPIC $(optymalizacja) -c ./libraries/library.c -o sharedLibrary.o

dynamicMain: libDynamicLibary.so
	gcc  $(optymalizacja) dynamicMain.c -ldl -o dynamicMain

libDynamicLibary.so: sharedLibrary.o
	gcc -shared -fPIC $(optymalizacja) -Wl,-soname,libDynamicLibrary.so.1 -o libDynamicLibrary.so.1.0.0 sharedLibrary.o -lc

testAll: sharedMain dynamicMain staticMain
	export LD_LIBRARY_PATH=`pwd`
	./target sharedMain >> $(wynik)
	./target dynamicMain >> $(wynik)
	./target staticMain >> $(wynik)


all: barber customer

barber: barber.o header.o
	gcc -o barber barber.o header.o -pthread -lrt

clean: cleanBarber.o cleanHeader.o cleanBarber

cleanBarber.o:
	rm barber.o

cleanHeader:
	rm header.o

cleanBarber:
	rm barber

customer: header.o customer.o
	gcc -o customer customer.o header.o -pthread -lrt

remove:
	rm *.o customer barber 
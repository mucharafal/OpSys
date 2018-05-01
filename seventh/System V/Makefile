all:
	gcc customer.c header.c -o customer
	gcc barber.c header.c -o barber

clean: rmbarber rmcustomer
	ipcrm --all=msg
	ipcrm --all=sem

rmbarber:
	rm barber

rmcustomer:
	rm customer
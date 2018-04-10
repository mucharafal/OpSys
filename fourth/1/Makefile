date:
	gcc printDate.c -o printDate

dateFork: printDateParent printDateChild

printDateChild:
	gcc printDateChild.c -o printDateChild

printDateParent:
	gcc printDateParent.c -o printDateParent

clean:
	rm printDate printDateChild printDateParent

cleanParentChild:
	rm printDateChild printDateParent
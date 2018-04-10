limits:
	gcc interpreterWithLimits.c -o interpreterWithLimits

interpreter:
	gcc interpreter.c -o interpreter

IHello:
	gcc IHello.c -o IHello

checkDirectory:
	gcc checkDirectory.c -o checkDirectory

infiniteLoop:
	gcc infiniteLoop.c -o infiniteLoop

recursion:
	gcc recursion.c -o recursion

toInterpret: interpreter IHello checkDirectory
	./interpreter ./.scripts/toInterpret

toInterpretLimits: limits IHello checkDirectory recursion infiniteLoop
	./interpreterWithLimits ./.scripts/toInterpret 100000000 1
	./interpreterWithLimits ./.scripts/recursion 100000000 1
	./interpreterWithLimits ./.scripts/infiniteLoop 100000000 1

clear:
	rm checkDirectory IHello infiniteLoop interpreter interpreterWithLimits recursion

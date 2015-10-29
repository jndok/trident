all:
	rm -rf bin;
	mkdir bin;
	gcc src/inject.c src/trident/trident.c -dynamiclib -Wl,-U,_main -o bin/inject.dylib	#	we need weak binding
	gcc src/test.c -o bin/test
clean:
	rm -rf bin

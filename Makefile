CC = gcc
CFLAGS = -g
OBJECTS = arraylist.o \
	hashtable.o \
	auxfuncs.o \
	str.o \
	main.o

clean:
	rm ./*.o
	if [ -f ./tachyon ]; then rm ./tachyon; fi

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

tachyon: $(OBJECTS)
	$(CC) $(OBJECTS) -o tachyon
	make post-build-cleanup

post-build-cleanup:
	rm ./*.o


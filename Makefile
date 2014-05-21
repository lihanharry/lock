

CC = gcc -g
CPPFLAGS = -g -O -Wall

TARGET1 = libsmartlock.a


all : $(TARGET1)  
$(TARGET1) : lock.cpp
	gcc -c  $^ $(CPPFLAGS) 
	ar -rcv $@ lock.o 
	rm -f *.o

clean:
	rm -f $(TARGET1) 
	rm -f *.o


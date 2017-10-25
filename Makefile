CC= g++
CPPFLAGS= -Wextra -Wall -g -std=c++11
HEADERS= uthread_core.h uthread_node.h uthreads.h uthread_util.h
TAR_FILES= $(HEADERS) uthread_core.cpp uthread_node.cpp uthreads.cpp uthread_util.cpp

all: libuthreads.a

libuthreads.a: uthread_core.o uthread_node.o uthreads.o uthread_util.o
	ar rcs libuthreads.a $^

%.o: %.cpp $(HEADERS)
	$(CC) $(CPPFLAGS) -c $<

tar:
	tar -cvf ex2.tar $(TAR_FILES) README Makefile

clean:
	rm -rf *.o libuthreads.a

.PHONY: clean all tar

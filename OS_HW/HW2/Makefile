CC=gcc

all:sched_demo_310512051

sched_demo_310512051: sched_demo_310512051.o
	$(CC) -o sched_demo_310512051 sched_demo_310512051.o 
	
sched_demo_310512051.o: sched_demo_310512051.c
	$(CC) -c -o sched_demo_310512051.o sched_demo_310512051.c -lpthread

clean:
	rm -f  sched_demo_310512051 sched_demo_310512051.*o

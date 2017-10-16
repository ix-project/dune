CC	= gcc
CFLAGS	= -Wall -g -MD -O2 -I ../
CXXFLAGS = -Wall -g -MD -O2 -std=c++11 -I ../

all: hello test timetest test_sandbox tlb_shootdown hugepages hugepages_mod.ko

hello: hello.o ../libdune/libdune.a
	$(CC) $(CFLAGS) $(<) -o hello -L ../libdune -ldune -lpthread

timetest: timetest.o ../libdune/libdune.a
	$(CC) $(CFLAGS) $(<) -o $(@) -L ../libdune -ldune -lpthread

test: test.o ../libdune/libdune.a
	$(CC) $(CFLAGS) $(<) -o test -L ../libdune -ldune -lpthread

tlb_shootdown: tlb_shootdown.o ../libdune/libdune.a
	$(CXX) $(CXXFLAGS) $(<) -o tlb_shootdown -L ../libdune -ldune -lpthread

test_sandbox: test_sandbox.o
	$(CC) $(CFLAGS) -o $(@) $(<) -lpthread

hugepages: hugepages.o ../libdune/libdune.a
	$(CC) $(CFLAGS) $(<) -o hugepages -L ../libdune -ldune

hugepages_mod.ko: hugepages_mod.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) obj-m=hugepages_mod.o CFLAGS= modules

clean:
	rm -f *.o test *.d hello test test_sandbox tlb_shootdown hugepages
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) obj-m=hugepages_mod.o clean

-include *.d

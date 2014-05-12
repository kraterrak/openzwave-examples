CC=g++

top_srcdir=/usr/local/include/openzwave/

INCLUDES:= -I $(top_srcdir) -I $(top_srcdir)command_classes -I $(top_srcdir)platform -I $(top_srcdir)value_classes
LIBS:= -lopenzwave -lpthread -ludev
LDFLAGS += -L/usr/local/lib -Wl,-R/usr/local/lib '-Wl,-R$$ORIGIN'

all:	ozw

ozw: Main.cpp
	$(CC) $(INCLUDES) -c Main.cpp -o ozw-pir-power-switch.o
	$(CC) $(LIBS) $(LDFLAGS) ozw-pir-power-switch.o -o ozw-pir-power-switch.out

clean:
	rm -rf *.o *.out

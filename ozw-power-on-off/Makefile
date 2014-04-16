CC=g++

top_srcdir=/usr/local/include/openzwave/

MKDIR_P= mkdir -p

INCLUDES:= -I $(top_srcdir) -I $(top_srcdir)command_classes -I $(top_srcdir)platform -I $(top_srcdir)value_classes
LIBS:= -lopenzwave -lpthread -ludev
LDFLAGS += -L/usr/local/lib -Wl,-R/usr/local/lib '-Wl,-R$$ORIGIN'

all:	meta ozw 

ozw: Main.cpp
	$(CC) $(INCLUDES) -c Main.cpp -o ozw-power-on-off.o
	$(CC) $(LIBS) $(LDFLAGS) ozw-power-on-off.o -o ozw-power-on-off.out

meta:
	$(MKDIR_P) ./meta

clean:
	rm -rf meta *.o *.out

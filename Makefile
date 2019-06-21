CFLAGS=-Wno-deprecated-declarations
LDLIBS=-lX11

OBJS=gridrun.o
BIN=gridrun

all: $(BIN) 

$(BIN): $(OBJS) Makefile

	gcc $(OBJS) $(LDLIBS) -o $(BIN)

clean:

	rm -f $(OBJS) $(BIN)


IDIR =../include
ODIR=obj
LDIR=../lib

#CC = gcc
#CFLAGS = -I $(IDIR) -Wall -Werror -pthread

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($(CFLAGS),)
	CFLAGS = -I $(IDIR) -Wall -Werror
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif


# #set of h files which .c files depend on
# _DEPS = gpio.h circularbuffer.h
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
#
# _OBJ = main.o
# OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: server

# # .o file depends on the .c version of the file and the .h files in DEPS
# # "-c" = generate object files,
# # "-o $@" = put output of compilation in the file name on the left side of ":"
# # "$<" = the first item in the dependencies list
# $(ODIR)/%.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

server: main.c
	$(CC) $(CFLAGS) $^ -o $@ -I $(IDIR) $(LDFLAGS) -pthread

.PHONY: clean

clean:
	rm -f server

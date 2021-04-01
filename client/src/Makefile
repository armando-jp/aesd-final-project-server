IDIR =../include
CC = gcc
CFLAGS = -I $(IDIR) -Wall -Werror

ODIR=obj
LDIR=../lib

LIBS=

#set of h files which .c files depend on
_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


# .o file depends on the .c version of the file and the .h files in DEPS
# "-c" = generate object files,
# "-o $@" = put output of compilation in the file name on the left side of ":"
# "$<" = the first item in the dependencies list
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ)
	$(CC) -o ../bin/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

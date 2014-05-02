CC       := clang
FLAGS    := -DNUM_CELLS=10 -DNUM_ROOTS=100 \
	        -std=c99
CPPFLAGS :=

.PHONY: clean

armstrong-virding: CFLAGS = ${FLAGS} -DGC=0
armstrong-virding: armstrong-virding.o \
                   lists.o shared.o main.o

fenichel-yochelson: CFLAGS = ${FLAGS} -DGC=1
fenichel-yochelson: fenichel-yochelson.o \
                    lists.o shared.o main.o

clean:
	-@rm *.o \
    armstrong-virding \
    fenichel-yochelson

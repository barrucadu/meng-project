CC = clang
CPPFLAGS :=

.PHONY: clean

armstrong-virding: CFLAGS = -DNUM_CELLS=10 -DNUM_ROOTS=100 -DGC=0 -std=c99
armstrong-virding: armstrong-virding.o lists.o shared.o main.o

fenichel-yochelson: CFLAGS = -DNUM_CELLS=10 -DNUM_ROOTS=100 -DGC=1 -std=c99
fenichel-yochelson: fenichel-yochelson.o lists.o shared.o main.o

clean:
	-@rm *.o armstrong-virding fenichel-yochelson

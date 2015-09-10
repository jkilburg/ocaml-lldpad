OCAMLC = ocamlc
CC = ocamlc
OCAMLMKLIB = ocamlmklib
GCC = gcc
OCAML = ocaml

all: llpdad.cma

llpdad.cma: llpdad.cmo Llpdad.so
        $(OCAMLC) -a -custom -o llpdad.cma llpdad.cmo -dllib llpdad.so -cclib -lllpdad

llpdad.cmo: llpdad.ml
        $(OCAMLC) llpdad.ml

llpdad.so: llpdad.o
        $(OCAMLMKLIB) -o llpdad clif_stub.o -lkrb5

clif_stub.o: clif_stub.c

OCAMLC = ocamlc
CC = ocamlc
OCAMLMKLIB = ocamlmklib
GCC = gcc
OCAML = ocaml

all: lldptool

lldp_clif.cma: lldp_clif.cmo dlllldp_clif.so
	$(OCAMLC) -a -custom -o lldp_clif.cma lldp_clif.cmo -dllib dlllldp_clif.so -cclib -llldp_clif

lldp_clif.cmo: lldp_clif.ml
	$(OCAMLC) lldp_clif.ml

dlllldp_clif.so: clif_stub.o
	$(OCAMLMKLIB) -o lldp_clif clif_stub.o -llldp_clif

lldptool: lldp_clif.cma
	$(OCAML) lldp_clif.cma lldptool.ml

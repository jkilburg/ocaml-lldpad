OCAMLC = ocamlc
CC = ocamlc
OCAMLMKLIB = ocamlmklib
GCC = gcc
OCAML = ocaml

all: lldptool lldpd

clean:
	rm -f *.o *.cmo *.cma *.a *.so a.out *.cmx *.cmi *.cmxa lldptool lldpd *~

#lldp_clif.cma: lldp_clif.cmo dlllldp_clif.so
#	$(OCAMLC) -a -custom -o lldp_clif.cma lldp_clif.cmo -dllib dlllldp_clif.so -cclib -llldp_clif

#lldp_clif.cmo: lldp_clif.ml
#	$(OCAMLC) lldp_clif.ml

#dlllldp_clif.so: clif_stub.o
#	$(OCAMLMKLIB) -o lldp_clif clif_stub.o -llldp_clif

clif.cmi: clif.mli Makefile
	ocamlfind ocamlopt -thread -g -c -package core -package async_kernel -package async $<

clif.cmx: clif.ml clif.cmi Makefile
	ocamlfind ocamlopt -thread -g -c -package core -package async_kernel -package async $<

lldptool: clif.cmx lldptool.ml Makefile
	ocamlfind ocamlopt -thread -g -o $@ -linkpkg -package core -package async_kernel -package async clif.cmx lldptool.ml

packet.cmx: packet.ml Makefile
	ocamlfind ocamlopt -thread -g -c -package core $<

netdevice.cmx: netdevice.ml Makefile
	ocamlfind ocamlopt -thread -g -c -package core $<

socket.cmx: socket.ml Makefile
	ocamlfind ocamlopt -thread -g -c -package core $<

liblldp.a: packet_intf.o netdevice_intf.o socket_intf.o netdevice.cmx packet.cmx socket.cmx Makefile
	$(OCAMLMKLIB) -custom -oc lldp packet_intf.o netdevice_intf.o netdevice.cmx packet.cmx socket.cmx

lldpd: lldpd.ml liblldp.a Makefile
	ocamlfind ocamlopt -thread -g -o $@ -linkpkg -package core -package async_kernel -package async lldpd.ml liblldp.a

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>

static value
result(value v, int ec)
{
  value r;
  if (ec == 0) {
    r = caml_alloc(1, 0);
    Store_field(r, 0, v);
  } else {
    r = caml_alloc(1, 1);
    Store_field(r, 0, Val_int(ec));
  }  
  return r;
}

#define RESULT(v, ec) CAMLreturn(result((v),(ec)))

#define UC(v) if ((v) < 0) { CAMLreturn(result(Val_unit, errno == 0 ? -1:errno)); }
  
CAMLprim value
packet_socket_c(value caml_protocol)
{
  CAMLparam1(caml_protocol);
  int fd;
  short protocol = htons(Int_val(caml_protocol));

  fd = socket(PF_PACKET, SOCK_RAW, protocol);
  if (fd < 0) CAMLreturn(result(Val_unit, errno == 0 ? -1:errno));

  RESULT(Val_int(fd), 0);
}

CAMLprim value
packet_bind_c(value caml_socket, value caml_ifindex, value caml_protocol)
{
  CAMLparam2(caml_ifindex, caml_protocol);
  struct sockaddr_ll ll;
  int socket = Int_val(caml_socket);
  int ifindex = Int_val(caml_ifindex);
  int protocol = htons(Int_val(caml_protocol));
  
  memset(&ll, 0, sizeof(ll));
  ll.sll_family = PF_PACKET;
  ll.sll_ifindex = ifindex;
  ll.sll_protocol = protocol;
  UC (bind(socket, (struct sockaddr *) &ll, sizeof(ll)));

  RESULT(Val_unit,0);
}

#define PACKET_MEMBERSHIP(OPERATION,FUNCNAME)				\
  CAMLprim value							\
  FUNCNAME(value caml_socket, value caml_ifindex, value caml_address)	\
  {									\
    CAMLparam3(caml_socket, caml_ifindex, caml_address);		\
    int socket = Int_val(caml_socket);					\
    int ifindex = Int_val(caml_ifindex);				\
    char *address = String_val(caml_address);				\
    int address_len = caml_string_length(caml_address);			\
    struct packet_mreq mr;						\
									\
    memset(&mr, 0, sizeof(mr));						\
    mr.mr_ifindex = ifindex;						\
    mr.mr_alen = address_len;						\
    memcpy(mr.mr_address, address, address_len);			\
    mr.mr_type = PACKET_MR_MULTICAST;					\
    UC (setsockopt(socket, SOL_PACKET, OPERATION, &mr, sizeof(mr)));	\
									\
    RESULT(Val_unit,0);							\
  }

PACKET_MEMBERSHIP(PACKET_ADD_MEMBERSHIP, packet_add_membership_c);
PACKET_MEMBERSHIP(PACKET_DROP_MEMBERSHIP, packet_drop_membership_c);

CAMLprim value
packet_origdev_c(value caml_socket, value caml_option)
{
  CAMLparam2(caml_socket, caml_option);
  int socket = Int_val(caml_socket);
  int option = Int_val(caml_option);

  UC (setsockopt(socket, SOL_PACKET, PACKET_ORIGDEV, &option, sizeof(option)));

  RESULT(Val_unit,0);
}

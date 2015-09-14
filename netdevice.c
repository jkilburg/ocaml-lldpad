#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <net/if.h>

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

#define FI(fd, r, ifr) if (ioctl((fd), (r), (ifr)) < 0) { CAMLreturn(result(Val_unit,errno == 0 ? -1:errno)); }

CAMLprim value
siocgifname_c(value caml_socket, value caml_ifindex)
{
  CAMLparam2(caml_socket, caml_ifindex);
  int socket = Int_val(caml_socket);
  int ifindex = Int_val(caml_ifindex);
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(struct ifreq));
  ifr.ifr_ifindex = ifindex;
  FI(socket, SIOCGIFNAME, &ifr);

  RESULT(caml_copy_string(ifr.ifr_name), 0);
}

CAMLprim value
siocgifindex_c(value caml_socket, value caml_ifname)
{
  CAMLparam2(caml_socket, caml_ifname);
  int socket = Int_val(caml_socket);
  char *ifname = String_val(caml_ifname);
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(struct ifreq));
  strcpy(ifr.ifr_name, ifname);
  FI(socket, SIOCGIFINDEX, &ifr);

  RESULT(Val_int(ifr.ifr_ifindex), 0);
}

#define SET_FLAGS(request, funcname)					\
  CAMLprim value							\
  funcname(value caml_socket, value caml_ifname, value caml_ifflags)	\
  {									\
    CAMLparam3(caml_socket, caml_ifname, caml_ifflags);			\
    int socket = Int_val(caml_socket);					\
    char *ifname = String_val(caml_ifname);				\
    short ifflags = Int_val(caml_ifflags);				\
    struct ifreq ifr;							\
									\
    memset(&ifr, 0, sizeof(struct ifreq));				\
    strcpy(ifr.ifr_name, ifname);					\
    ifr.ifr_flags = ifflags;						\
    FI(socket, request, &ifr);						\
    RESULT(Val_int(ifr.ifr_flags), 0);					\
  }

SET_FLAGS(SIOCSIFFLAGS, siocsifflags_c)
SET_FLAGS(SIOCSIFPFLAGS, siocsifpflags_c)

#define GET_FLAGS(request, funcname)			\
  CAMLprim value					\
  funcname(value caml_socket, value caml_ifname)	\
  {							\
    CAMLparam2(caml_socket, caml_ifname);		\
    int socket = Int_val(caml_socket);			\
    char *ifname = String_val(caml_ifname);		\
    struct ifreq ifr;					\
							\
    memset(&ifr, 0, sizeof(struct ifreq));		\
    strcpy(ifr.ifr_name, ifname);			\
    FI(socket, request, &ifr);				\
    RESULT(Val_int(ifr.ifr_flags), 0);			\
  }

GET_FLAGS(SIOCGIFFLAGS, siocgifflags_c)
GET_FLAGS(SIOCGIFPFLAGS, siocgifpflags_c)


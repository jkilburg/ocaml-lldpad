#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>

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

#define IFCONF_MAXLEN 32

CAMLprim value
siocgifconf_c(value caml_socket)
{
  CAMLparam1(caml_socket);
  int socket = Int_val(caml_socket);
  struct ifreq ifr[IFCONF_MAXLEN];
  struct ifconf ifc;
  int i;
  value caml_iflist = Val_int(0);
  value new_if;
  value pair;
  struct sockaddr_in *sin;
  
  ifc.ifc_len = IFCONF_MAXLEN;
  ifc.ifc_req = ifr;
  
  FI(socket, SIOCGIFCONF, &ifc);
  
  for (i = 0; i < ifc.ifc_len; i++) {
    /* ifname:string * ipaddr:int32 */
    pair = caml_alloc(2, 0);
    Store_field(pair, 0, caml_copy_string(ifc.ifc_req[i].ifr_name));
    sin = (struct sockaddr_in *)&ifc.ifc_req[i].ifr_addr;
    Store_field(pair, 1, caml_copy_int32(sin->sin_addr.s_addr));

    /* next list node */
    new_if = caml_alloc(2, 0);
    Store_field(new_if, 0, pair);
    Store_field(new_if, 1, caml_iflist);
    caml_iflist = new_if;
  }
  
  RESULT(caml_iflist, 0);
}

static void
copyifname(char *dst, char *src)
{
  strncpy(dst, src, IFNAMSIZ);
  dst[IFNAMSIZ - 1] = '\0';
  return;
}

#define GET_FIELD(REQUEST, FUNCNAME, GETTER)		\
  CAMLprim value					\
  FUNCNAME(value caml_socket, value caml_ifname)	\
  {							\
    CAMLparam2(caml_socket, caml_ifname);		\
    int socket = Int_val(caml_socket);			\
    char *ifname = String_val(caml_ifname);		\
    struct ifreq ifr;					\
							\
    memset(&ifr, 0, sizeof(struct ifreq));		\
    copyifname(ifr.ifr_name, ifname);			\
    FI(socket, REQUEST, &ifr);				\
    RESULT(GETTER, 0);					\
  }

GET_FIELD(SIOCGIFINDEX, siocgifindex_c, Val_int(ifr.ifr_ifindex))
GET_FIELD(SIOCGIFMTU, siocgifmtu_c, Val_int(ifr.ifr_mtu))
GET_FIELD(SIOCGIFFLAGS, siocgifflags_c, Val_int(ifr.ifr_flags))
GET_FIELD(SIOCGIFPFLAGS, siocgifpflags_c, Val_int(ifr.ifr_flags))
GET_FIELD(SIOCGIFTXQLEN, siocgiftxqlen_c, Val_int(ifr.ifr_qlen))

#define ETHERNET_MAC_LEN 6

static value
get_hwaddr(struct ifreq *ifr)
{
  int len;
  value addr;

  if (ifr->ifr_hwaddr.sa_family == ARPHRD_ETHER) len = ETHERNET_MAC_LEN;
  else caml_failwith("UNSUPPORTED HARDWARE IN SIOCGIFHWADDR");
 
  addr = caml_alloc_string(len);
  memcpy(String_val(addr), ifr->ifr_hwaddr.sa_data, len);
  return addr;
}

GET_FIELD(SIOCGIFHWADDR, siocgifhwaddr_c, get_hwaddr(&ifr))

GET_FIELD(SIOCGIFADDR, siocgifaddr_c, caml_copy_int32(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr))
GET_FIELD(SIOCGIFBRDADDR, siocgifbrdaddr_c, caml_copy_int32(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr.s_addr))
GET_FIELD(SIOCGIFNETMASK, siocgifnetmask_c, caml_copy_int32(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr))

#define SET_FIELD(REQUEST, FUNCNAME, SETTER)				\
  CAMLprim value							\
  FUNCNAME(value caml_socket, value caml_ifname, value caml_val)	\
  {									\
    CAMLparam3(caml_socket, caml_ifname, caml_val);			\
    int socket = Int_val(caml_socket);					\
    char *ifname = String_val(caml_ifname);				\
    struct ifreq ifr;							\
    									\
    memset(&ifr, 0, sizeof(struct ifreq));				\
    copyifname(ifr.ifr_name, ifname);					\
    SETTER;								\
    FI(socket, REQUEST, &ifr);						\
    RESULT(Val_unit, 0);						\
  }

SET_FIELD(SIOCSIFFLAGS, siocsifflags_c, ifr.ifr_flags = Int_val(caml_val))
SET_FIELD(SIOCSIFPFLAGS, siocsifpflags_c, ifr.ifr_flags = Int_val(caml_val))
SET_FIELD(SIOCSIFMTU, siocsifmtu_c, ifr.ifr_mtu = Int_val(caml_val))
SET_FIELD(SIOCSIFTXQLEN, siocsiftxqlen_c, ifr.ifr_qlen = Int_val(caml_val))

SET_FIELD(SIOCSIFNAME, siocsifname_c, copyifname(ifr.ifr_newname, String_val(caml_val)))

static void
set_hwaddr(struct sockaddr *sa, value hwaddr)
{
  /* quick and dirty checks */
  if (caml_string_length(hwaddr) != ETHERNET_MAC_LEN) caml_failwith("Expected 6 byte ethernet MAC");
  memcpy(sa->sa_data, String_val(hwaddr), ETHERNET_MAC_LEN);
  return;
}

SET_FIELD(SIOCSIFHWADDR, siocsifhwaddr_c, set_hwaddr(&ifr.ifr_hwaddr, caml_val))

static void
set_ipaddr(struct sockaddr *sa, value ipaddr)
{
  struct sockaddr_in *sin = (struct sockaddr_in *)sa;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = Int32_val(ipaddr);
  return;
}

SET_FIELD(SIOCSIFADDR, siocsifaddr_c, set_ipaddr(&ifr.ifr_addr, caml_val))
SET_FIELD(SIOCSIFBRDADDR, siocsifbrdaddr_c, set_ipaddr(&ifr.ifr_broadaddr, caml_val))
SET_FIELD(SIOCSIFNETMASK, siocsifnetmask_c, set_ipaddr(&ifr.ifr_netmask, caml_val))

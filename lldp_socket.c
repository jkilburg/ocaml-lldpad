/* swiped from lldpad but modified. */

/*******************************************************************************

  LLDP Agent Daemon (LLDPAD) Software
  Copyright(c) 2007-2012 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  open-lldp Mailing List <lldp-devel@open-lldp.org>

*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <linux/pkt_sched.h>
#include <net/if.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>

#define ETH_P_LLDP 0x88cc

static const char nearest_bridge[ETH_ALEN] = {0x01,0x80,0xc2,0x00,0x00,0x0e};
static const char nearest_nontpmr_bridge[ETH_ALEN] = {0x01,0x80,0xc2,0x00,0x00,0x03};
static const char nearest_customer_bridge[ETH_ALEN] = {0x01,0x80,0xc2,0x00,0x00,0x00};

static value
result(int fd, int ec)
{
  CAMLlocal1(r);
  r = caml_alloc(2,0);
  Store_field(r, 0, Val_int(fd));
  Store_field(r, 1, Val_int(ec));
  return r;
}

CAMLprim value
lldp_socket_c(value caml_ifname)
{
  CAMLparam1(caml_ifname);
  int fd;
  char *ifname = String_val(caml_ifname);
  struct ifreq ifr;
  struct sockaddr_ll ll;
  int ifindex;
  short protocol = htons(ETH_P_ARP);

  fd = socket(PF_PACKET, SOCK_RAW, protocol);
  if (fd < 0) CAMLreturn(result(fd, errno));

#define UC(v) if ((v) < 0) { value r = result(-1,errno); close(fd); CAMLreturn(r); }
  
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  UC (ioctl(fd, SIOCGIFINDEX, &ifr))

  ifindex = ifr.ifr_ifindex;

  memset(&ll, 0, sizeof(ll));
  ll.sll_family = PF_PACKET;
  ll.sll_ifindex = ifr.ifr_ifindex;
  ll.sll_protocol = protocol;
  UC (bind(fd, (struct sockaddr *) &ll, sizeof(ll)));

  struct packet_mreq mr;
  memset(&mr, 0, sizeof(mr));
  mr.mr_ifindex = ifindex;
  mr.mr_alen = ETH_ALEN;
  memcpy(mr.mr_address, &nearest_bridge, ETH_ALEN);
  mr.mr_type = PACKET_MR_MULTICAST;
  UC (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)));

  memcpy(mr.mr_address, &nearest_customer_bridge, ETH_ALEN);
  UC (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)));

  memcpy(mr.mr_address, &nearest_nontpmr_bridge, ETH_ALEN);
  UC (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)));

  int option = 1;
  int option_size = sizeof(option);
  UC (setsockopt(fd, SOL_PACKET, PACKET_ORIGDEV, &option, option_size));

  option = TC_PRIO_CONTROL;
  UC (setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &option, option_size));

  CAMLreturn(result(fd,0));
}

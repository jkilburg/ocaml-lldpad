#include <stdio.h>
#include <sys/types.h>

#include <lldpad/clif.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>

/*
 * No fancy type handling for now.
 */
static struct custom_operations lldpad_clif_default_ops = {
  "com.greenjeans.caml.lldpad.clif",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

#define Clif_val(v) (*((struct clif **) Data_custom_val(v)))

/* swiped from http://www.linux-nantes.org/~fmonnier/OCaml/ocaml-wrapping-c.html */

#define Val_none Val_int(0)

static value
Val_some(value v)
{
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc(1, 0);
  Store_field(some, 0, v);
  CAMLreturn(some);
}

CAMLprim value
clif_open_stub(value unit)
{
  CAMLparam1(unit);
  CAMLlocal1(caml_clif);
  struct clif *clif;
  
  clif = clif_open();
  if (clif == NULL) CAMLreturn (Val_none);

  caml_clif = alloc_custom(&lldpad_clif_default_ops, sizeof(struct clif *), 0, 1);  
  Clif_val(caml_clif) = clif;
  
  CAMLreturn(Val_some(caml_clif));
}

CAMLprim value
clif_close_stub(value caml_clif)
{
  CAMLparam1(caml_clif);

  clif_close(Clif_val(caml_clif));

  CAMLreturn(Val_unit);
}

CAMLprim value
clif_request_stub(value caml_clif, value caml_cmd, value caml_reply)
{
  CAMLparam3(caml_clif, caml_cmd, caml_reply);
  int retval;
  char *cmd = String_val(caml_cmd);
  size_t cmd_len = caml_string_length(caml_cmd);
  char *reply = String_val(caml_reply);
  size_t reply_len = caml_string_length(caml_reply);

  retval = clif_request(Clif_val(caml_clif), cmd, cmd_len, reply, &reply_len, NULL);

  /* we truncate reply_len here but we will not have a reply that exceeds 2^31 in length
     and we overload the return value a bit. */
  if (retval == 0) CAMLreturn(Val_int(reply_len));
  
  CAMLreturn(Val_int(retval));
}

CAMLprim value
clif_attach_stub(value caml_clif, value caml_hex_tlvs)
{
  CAMLparam2(caml_clif, caml_hex_tlvs);
  int retval;
  char *hex_tlvs = String_val(caml_hex_tlvs);

  retval = clif_attach(Clif_val(caml_clif), hex_tlvs);

  CAMLreturn(Val_int(retval));
}

CAMLprim value
clif_detach_stub(value caml_clif)
{
  CAMLparam1(caml_clif);
  int retval;

  retval = clif_detach(Clif_val(caml_clif));

  CAMLreturn(Val_int(retval));
}

CAMLprim value
clif_recv_stub(value caml_clif, value caml_reply)
{
  CAMLparam2(caml_clif, caml_reply);
  int retval;
  char *reply = String_val(caml_reply);
  size_t reply_len = caml_string_length(caml_reply);

  retval = clif_recv(Clif_val(caml_clif), reply, &reply_len);
  
  /* we truncate reply_len here but we will not have a reply that exceeds 2^31 in length
     and we overload the return value a bit. */
  if (retval == 0) CAMLreturn(Val_int(reply_len));
  
  CAMLreturn(Val_int(retval));
}

CAMLprim value
clif_pending_stub(value caml_clif)
{
  CAMLparam1(caml_clif);
  int retval;

  retval = clif_pending(Clif_val(caml_clif));
  
  CAMLreturn(Val_int(retval));
}

CAMLprim value
clif_pending_waiting_stub(value caml_clif, value caml_waittime)
{
  CAMLparam2(caml_clif, caml_waittime);
  int retval;

  retval = clif_pending_wait(Clif_val(caml_clif), Int_val(caml_waittime));
  
  CAMLreturn(Val_int(retval));
}

CAMLprim value
clif_get_fd_stub(value caml_clif)
{
  CAMLparam1(caml_clif);
  int fd;

  fd = clif_get_fd(Clif_val(caml_clif));

  CAMLreturn(Val_int(fd));
}

CAMLprim value
clif_getpid_stub(value unit)
{
  CAMLparam1(unit);
  pid_t pid;

  pid = clif_getpid();

  CAMLreturn(Val_int(pid));
}

CAMLprim value
clif_vsi_stub(value caml_clif, value caml_ifname, value caml_tlvid, value caml_cmd, value caml_reply)
{
  CAMLparam5(caml_clif, caml_ifname, caml_tlvid, caml_cmd, caml_reply);
  CAMLlocal1(caml_result);
  int retval;
  unsigned int tlvid = Int_val(caml_tlvid);
  char *ifname = String_val(caml_ifname);
  char *cmd = String_val(caml_cmd);
  char *reply = String_val(caml_reply);
  size_t reply_len = caml_string_length(caml_reply);

  retval = clif_vsi(Clif_val(caml_clif), ifname, tlvid, cmd, reply, &reply_len);

  caml_result = caml_alloc(2, 0);
  Store_field(caml_result, 0, Val_int(retval));
  Store_field(caml_result, 1, Val_int(reply_len));

  CAMLreturn (caml_result);
}

CAMLprim value
clif_vsievt_stub(value caml_clif, value caml_reply, value caml_waittime)
{
  CAMLparam3(caml_clif, caml_reply, caml_waittime);
  CAMLlocal1(caml_result);
  int retval;
  int waittime = Int_val(caml_waittime);
  char *reply = String_val(caml_reply);
  size_t reply_len = caml_string_length(caml_reply);

  retval = clif_vsievt(Clif_val(caml_clif), reply, &reply_len, waittime);

  caml_result = caml_alloc(2, 0);
  Store_field(caml_result, 0, Val_int(retval));
  Store_field(caml_result, 1, Val_int(reply_len));

  CAMLreturn (caml_result);
}

CAMLprim value
clif_vsiwait_stub(value caml_clif, value caml_ifname, value caml_tlvid,
		  value caml_cmd, value caml_reply, value caml_waittime)
{
  CAMLparam5(caml_clif, caml_ifname, caml_tlvid, caml_cmd, caml_reply);
  CAMLxparam1(caml_waittime);
  CAMLlocal1(caml_result);
  int retval;
  int waittime = Int_val(caml_waittime);
  unsigned int tlvid = Int_val(caml_tlvid);
  char *ifname = String_val(caml_ifname);
  char *cmd = String_val(caml_cmd);
  char *reply = String_val(caml_reply);
  size_t reply_len = caml_string_length(caml_reply);

  retval = clif_vsiwait(Clif_val(caml_clif), ifname, tlvid, cmd, reply, &reply_len, waittime);

  caml_result = caml_alloc(2, 0);
  Store_field(caml_result, 0, Val_int(retval));
  Store_field(caml_result, 1, Val_int(reply_len));

  CAMLreturn (caml_result);
}

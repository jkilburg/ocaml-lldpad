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

CAMLprim value
clif_open_stub(value unit)
{
  CAMLparam1(unit);
  CAMLlocal1(caml_clif);
  struct clif *clif;
  
  clif = clif_open();
  
  caml_clif = alloc_custom(&lldpad_clif_default_ops, sizeof(struct clif *), 0, 1);
  Clif_val(caml_clif) = clif;
  
  CAMLreturn(caml_clif);
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
  int cmd_len = caml_string_length(caml_cmd);
  char *reply = String_val(caml_reply);
  size_t reply_len = caml_string_length(caml_reply);
  
  retval = clif_request(Clif_val(caml_clif), cmd, cmd_len, reply, &reply_len, NULL);

  CAMLreturn(Val_int(retval));
}

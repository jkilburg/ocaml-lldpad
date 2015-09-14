#include <stdio.h>
#include <errno.h>

#include <sys/socket.h>

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
socket_so_priority(value caml_socket, value caml_option)
{
  CAMLparam2(caml_socket, caml_option);
  int socket = Int_val(caml_socket);
  int option = Int_val(caml_option);

  UC (setsockopt(socket, SOL_SOCKET, SO_PRIORITY, &option, sizeof(option)));
  
  RESULT(Val_unit,0);
}

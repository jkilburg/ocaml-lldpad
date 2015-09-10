open Lldp_clif

let () =
  let clif = clif_open() in
  clif_close(clif)
;;

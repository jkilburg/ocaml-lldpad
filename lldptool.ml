open Lldp_clif
open Printf

let () =
  match clif_open () with
  | None -> printf "clif_open failed\n"
  | Some clif ->
    begin
      let reply = Bytes.create (1024 * 1024 * 10) in
      match clif_request clif "PING" reply with
      | -1 | -2 -> printf "clif_request failed\n"
      | x ->        
        printf "clif_request reply: %s\n" (Bytes.sub_string reply 0 x)
    end;
    clif_close clif
;;

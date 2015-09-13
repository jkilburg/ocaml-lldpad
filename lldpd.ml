open Core.Std
open Async.Std

module CU = Core.Std.Unix

let buflen = 10 * 1024

let recv fd buf =
  In_thread.run ~name:"lldp"
    (fun () -> CU.recv fd ~buf ~pos:0 ~len:buflen ~mode:[])
;;

let rec read_all fd buf =
  recv fd buf
  >>= function
  | 0 -> Deferred.Or_error.error_string "Unexpected zero length packet"
  | res when res < 0 -> Deferred.Or_error.errorf "recv err %d" res
  | res ->
    printf "PACKET:\n";
    Bytes.sub_string buf 0 res
    |> String.to_list
    |> List.iter ~f:(fun c -> printf "%02x\n" (Char.to_int c));
    read_all fd buf
;;

let main () =
  Command.async_or_error
    ~summary:"Print LLDP packets"
    Command.Spec.(
      empty
    )
    (fun () ->
       match Lldp.socket "enp4s0" with
       | (fd,0) ->
         let buf = Bytes.create buflen in
         read_all (CU.File_descr.of_int fd) buf
       | (_,errno) -> Deferred.Or_error.errorf "Failed with errno: %d\n" errno)
;;

let () = Command.run (main ())

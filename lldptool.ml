open Core.Std
open Async.Std

open Clif

let rec read_events clif buf =
  clif_recv clif buf
  >>= function
  | Ok len ->
    let str = Bytes.sub_string buf 0 len in
    printf "Response: %s\n" str;
    read_events clif buf
  | Error e ->
    eprintf "clif_recv error: %s\n" (Error.to_string_hum e);
    Deferred.unit
;;

let main () =
  clif_open ()
  >>=? fun clif ->
  clif_attach clif [1;2;3;4;5;6;7;8;9;10;11]
  >>=? fun () ->
  let buf = Bytes.create 100 in
  read_events clif buf
  >>= fun _ ->
  clif_detach clif
  >>= function
  | Error e -> Error.raise e
  | Ok ()   -> Shutdown.exit 0
;;

let () =
  ignore (main ());
  never_returns (Scheduler.go ())
;;

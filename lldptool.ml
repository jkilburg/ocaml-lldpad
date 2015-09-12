open Core.Std
open Async.Std

open Clif

let main () =
  clif_open ()
  >>=? fun clif ->
  clif_attach clif []
  >>=? fun () ->
  clif_detach clif
  >>= function
  | Error e -> Error.raise e
  | Ok ()   -> Shutdown.exit 0
;;

let () =
  ignore (main ());
  never_returns (Scheduler.go ())
;;

open Core.Std
open Async.Std

module CU = Core.Std.Unix
              
type t = {
  socket       : UnixLabels.file_descr;
  connect_addr : CU.sockaddr;
  bind_addr    : CU.sockaddr;
}

let connect_socket_name () =
  (Char.of_int_exn 0 |> String.of_char)
  ^ "/com/intel/lldpad"
;;

let bind_socket_name () =
  (Char.of_int_exn 0 |> String.of_char)
  ^ sprintf "/com/intel/lldpad/%d" (CU.getpid () |> Pid.to_int)
;;

let make_socket bind_addr connect_addr =
  In_thread.run ~name:"clif"
    (fun () ->
       Or_error.try_with
         (fun () ->
            let socket =
              CU.socket
                ~kind:CU.SOCK_DGRAM
                ~domain:CU.PF_UNIX
                ~protocol:0
            in
            CU.bind socket ~addr:bind_addr;
            CU.connect socket ~addr:connect_addr;
            socket))
;;

let clif_open () =
  let bind_addr = CU.ADDR_UNIX (bind_socket_name ()) in
  let connect_addr = CU.ADDR_UNIX (connect_socket_name ()) in
  make_socket bind_addr connect_addr
  >>=? fun socket ->
  Deferred.Or_error.return { socket; connect_addr; bind_addr }
;;

let clif_close t =
  CU.close t.socket;
  Deferred.unit
;;

let send t buf =
  In_thread.run ~name:"clif"
    (fun () -> Or_error.try_with (fun () -> CU.send t.socket ~buf ~pos:0 ~len:(Bytes.length buf) ~mode:[]))
;;

let recv t buf =
  In_thread.run ~name:"clif"
    (fun () -> Or_error.try_with (fun () -> CU.recv t.socket ~buf ~pos:0 ~len:(Bytes.length buf) ~mode:[]))
;;

let rec clif_request ?f t cmd reply =
  send t cmd
  >>=? fun _ ->
  recv t reply
  >>=? function
  | 0   -> Deferred.Or_error.error_string "Unexpected reply of length zero."
  | res when res < 0 -> Deferred.Or_error.errorf "Error return code %d" res
  | res ->
    let is_event_msg =
      match Bytes.get reply 0 with
      | 'M' -> res > 9 && Bytes.get reply 9 = 'E'
      | 'E' -> true
      | _   -> false
    in
    if is_event_msg
    then
      begin
        match f with
        | None         -> clif_request ?f t cmd reply
        | Some f_noopt -> f_noopt reply res >>= fun () -> clif_request ?f t cmd reply
      end
    else Deferred.Or_error.return res
;;

let helper t cmd =
  let reply = Bytes.create 10 in
  clif_request t cmd reply
  >>= function
  | Error _ as e     -> return e
  | Ok x when x >= 3 ->
    let str = Bytes.sub_string reply 0 x in
    if String.is_prefix ~prefix:"R00" str
    then Deferred.Or_error.ok_unit
    else Deferred.Or_error.errorf "Expected response R00 got %s" str
  | Ok x             -> Deferred.Or_error.errorf "Expected 3 byte response, got %d" x
;;

let clif_detach t = helper t "D" ;;

let clif_attach t _tlvs = helper t "A" ;;

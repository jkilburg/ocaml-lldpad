open Core.Std
open Async.Std

type t = {
  socket : ([ `Active ], Socket.Address.Unix.t) Socket.t;
  reader : Reader.t;
  writer : Writer.t;
}

let connect_socket_name () =
  (Char.of_int_exn 0 |> String.of_char)
  ^ "/com/intel/lldpad"
;;

let bind_socket_name () =
  (Char.of_int_exn 0 |> String.of_char)
  ^ sprintf "/com/intel/lldpad/%d" (Unix.getpid () |> Pid.to_int)
;;

let make_socket () =
  let bind_addr = Socket.Address.Unix.create (bind_socket_name ()) in
  let connect_addr = Socket.Address.Unix.create (connect_socket_name ()) in
  let socket = Socket.create Socket.Type.unix_dgram in
  Socket.bind socket bind_addr
  >>= fun socket ->
  Socket.connect socket connect_addr
;;

(* snarfed from async_extra *)
let reader_writer_of_sock ?buffer_age_limit ?reader_buffer_size s =
  let fd = Socket.fd s in
  (Reader.create ?buf_len:reader_buffer_size fd,
   Writer.create ?buffer_age_limit fd)
;;

let clif_open () =
  make_socket ()
  >>= fun socket ->
  let (reader,writer) = reader_writer_of_sock socket in
  Deferred.Or_error.return { socket; reader; writer }
;;

let clif_close t =
  Socket.shutdown t.socket `Both;
  Deferred.Or_error.ok_unit
;;

let rec clif_request ?f t cmd reply =
  Writer.write t.writer cmd;
  Reader.read t.reader reply
  >>= function
  | `Eof  -> Deferred.Or_error.error_string "Unexpected EOF reading clif socket."
  | `Ok res ->
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
  | Error _ as e -> return e
  | Ok x ->
    let str = Bytes.sub_string reply 0 x in
    if String.is_prefix ~prefix:"R00" str
    then Deferred.Or_error.ok_unit
    else Deferred.Or_error.errorf "Expected response starting with R00 got %s" str
;;

let clif_detach t = helper t "D" ;;

let clif_attach t tlvs =
  List.map tlvs ~f:(sprintf "%02x")
  |> String.concat ~sep:","
  |> sprintf "A%s"
  |> helper t
;;

let clif_recv t reply =
  Reader.read t.reader reply
  >>= function
  | `Eof -> Deferred.Or_error.error_string "Unexpected EOF"
  | `Ok res -> Deferred.Or_error.return res
;;

let clif_pending t =
  Fd.ready_to (Socket.fd t.socket) `Read
  >>= function
  | `Ready -> Deferred.Or_error.return true
  | `Bad_fd -> Deferred.Or_error.error_string "Bad socket file descriptor."
  | `Closed -> Deferred.Or_error.error_string "Socket closed."
;;

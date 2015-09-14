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

let nearest_bridge = [ 0x01; 0x80; 0xc2; 0x00; 0x00; 0x0e ]
let nearest_nontpmr_bridge = [ 0x01; 0x80; 0xc2; 0x00; 0x00; 0x03 ]
let nearest_customer_bridge = [ 0x01; 0x80; 0xc2; 0x00; 0x00; 0x00 ]
    
let lldp_protocol_number = 0x88cc
let arp_protocol_number = 0x0806
let protocol_number = arp_protocol_number

let lb l =
  let b = Bytes.create (List.length l) in
  List.iteri l ~f:(fun p i -> Bytes.set b p (Char.of_int_exn i));
  b
;;

let setup_socket () =
  let open Core.Std.Or_error in
  let error msg = function
    | Error ec  -> Or_error.errorf "%s Errno = %d" msg ec
    | Ok _ as z -> z
  in
  error "Packet.socket" (Packet.socket protocol_number)
  >>= fun fd ->
  error "Netdevice.siocgifindex" (Netdevice.siocgifindex fd "enp4s0")
  >>= fun ifindex ->
  error "Packet.bind" (Packet.bind fd ifindex protocol_number)
  >>= fun () ->
  error "Netdevice.siocgifhwaddr" (Netdevice.siocgifhwaddr fd "enp4s0")
  >>= fun hw ->
  error "Packet.add_membership" (Packet.add_membership fd ifindex (lb nearest_bridge))
  >>= fun () ->
  error "Packet.add_membership" (Packet.add_membership fd ifindex (lb nearest_nontpmr_bridge))
  >>= fun () ->
  error "Packet.add_membership" (Packet.add_membership fd ifindex (lb nearest_customer_bridge))
  >>= fun () ->
  printf "Interface MAC: ";
  Bytes.iter (fun c -> printf "%02x" (Char.to_int c)) hw;
  printf "\n";
  return fd
;;

let main () =
  Command.async_or_error
    ~summary:"Print LLDP packets"
    Command.Spec.(
      empty
    )
    (fun () ->
       match setup_socket () with
       | Error _ as e -> return e
       | Ok fd ->
         let buf = Bytes.create buflen in
         read_all (CU.File_descr.of_int fd) buf)
;;

let () = Command.run (main ())

open Core.Std

external socket          : int -> (int,int) Result.t                  = "packet_socket_c"
external bind            : int -> int -> int -> (unit,int) Result.t   = "packet_bind_c"
external add_membership  : int -> int -> bytes -> (unit,int) Result.t = "packet_add_membership_c"
external drop_membership : int -> int -> bytes -> (unit,int) Result.t = "packet_drop_membership_c"
external origdev         : int -> int -> (unit,int) Result.t          = "packet_origdev_c"

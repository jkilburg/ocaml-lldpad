
val siocgifname   : int -> int -> (string,int) Result.t          = "siocgifname_c"
val siocgifindex  : int -> string -> (int,int) Result.t          = "siocgifindex_c"                                    
val siocgifflags  : int -> string -> (int,int) Result.t          = "siocgifflags_c"
val siocsifflags  : int -> string -> int -> (unit,int) Result.t  = "siocsifflags_c"
val siocgifpflags : int -> string -> (int,int) Result.t          = "siocgifpflags_c"
val siocsifpflags : int -> string -> int -> (unit,int) Result.t  = "siocsifpflags_c"
val siocgifaddr   : int -> string -> 
val soicsifaddr   : int -> string ->

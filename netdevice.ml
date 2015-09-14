open Core.Std
       
external siocgifname    : int -> int -> (string,int) Result.t   = "siocgifname_c"
external siocgifindex   : int -> string -> (int,int) Result.t   = "siocgifindex_c"                                    
external siocgifflags   : int -> string -> (int,int) Result.t   = "siocgifflags_c"
external siocgifpflags  : int -> string -> (int,int) Result.t   = "siocgifpflags_c"
external siocgifmtu     : int -> string -> (int,int) Result.t   = "siocgifmtu_c"
external siocgifaddr    : int -> string -> (int32,int) Result.t = "siocgifaddr_c"
external siocgifbrdaddr : int -> string -> (int32,int) Result.t = "siocgifbrdaddr_c"
external siocgifnetmask : int -> string -> (int32,int) Result.t = "siocgifnetmask_c"
external siocgifhwaddr  : int -> string -> (bytes,int) Result.t = "siocgifhwaddr_c"
  
external siocgifconf : int -> ((string * int32) list, int) Result.t = "siocgifconf_c"
  
external siocsifflags  : int -> string -> int -> (unit,int) Result.t  = "siocsifflags_c"
external siocsifpflags : int -> string -> int -> (unit,int) Result.t  = "siocsifpflags_c"

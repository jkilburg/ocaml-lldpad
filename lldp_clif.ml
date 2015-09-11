
type clif
  
external clif_open         : unit -> clif option             = "clif_open_stub"
external clif_close        : clif -> unit                    = "clif_close_stub"
external clif_request      : clif -> string -> string -> int = "clif_request_stub"
external clif_attach       : clif -> string -> int           = "clif_attach_stub"
external clif_detach       : clif -> int                     = "clif_detach_stub"
external clif_recv         : clif -> string -> (int * int)   = "clif_recv_stub"
external clif_pending      : clif -> int                     = "clif_pending_stub"
external clif_pending_wait : clif -> int -> int              = "clif_pending_wait_stub"
external clif_get_fd       : clif -> int                     = "clif_get_fd_stub"
external clif_getpid       : unit -> int                     = "clif_getpid_stub"
external clif_vsi          : clif -> string -> int -> string -> string -> (int * int) = "clif_vsi_stub"
external clif_vsievt       : clif -> string -> int -> (int * int) = "clif_vsievt_stub"
external clif_vsiwait      : clif -> string -> int -> string -> string -> int -> (int * int) = "clif_vsiwait_stub"

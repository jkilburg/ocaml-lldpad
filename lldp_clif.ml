
type clif
  
external clif_open    : unit -> clif                  = "clif_open_stub"
external clif_close   : clif -> unit                  = "clif_close_stub"
external clif_request : clif * string * string -> int = "clif_request_stub"

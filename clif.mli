open Core.Std
open Async.Std

type t

val clif_open    : unit -> t Deferred.Or_error.t
val clif_close   : t -> unit Deferred.Or_error.t
val clif_attach  : t -> int list -> unit Deferred.Or_error.t
val clif_detach  : t -> unit Deferred.Or_error.t
val clif_recv    : t -> bytes -> int Deferred.Or_error.t
val clif_request : ?f:(bytes -> int -> unit Deferred.t) -> t -> bytes -> bytes -> int Deferred.Or_error.t
val clif_pending : t -> bool Deferred.Or_error.t

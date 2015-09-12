open Core.Std
open Async.Std

type t

val clif_open   : unit -> t Deferred.Or_error.t
val clif_close  : t -> unit Deferred.t
val clif_attach : t -> bytes list -> unit Deferred.Or_error.t
val clif_detach : t -> unit Deferred.Or_error.t

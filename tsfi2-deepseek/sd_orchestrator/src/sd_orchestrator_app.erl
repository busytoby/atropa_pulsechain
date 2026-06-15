-module(sd_orchestrator_app).
-behaviour(application).

-export([start/2, stop/1]).

start(_StartType, _StartArgs) ->
    sd_orchestrator_sup:start_link().

stop(_State) ->
    ok.

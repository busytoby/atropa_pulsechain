-module(sd_orchestrator_sup).
-behaviour(supervisor).

-export([start_link/0]).
-export([init/1]).

-define(SERVER, ?MODULE).

start_link() ->
    supervisor:start_link({local, ?SERVER}, ?MODULE, []).

init([]) ->
    SupFlags = #{strategy => one_for_one,
                 intensity => 5,
                 period => 10},
                 
    %% Discover CCX list dynamically
    CcxList = case cpu_topology:discover_ccxs() of
        {ok, Lists} -> Lists;
        _ -> ["0"]
    end,
    
    %% Generate child specs for each CCX group
    ChildSpecs = lists:map(fun({_CcxListStr, Idx}) ->
        WorkerId = list_to_atom("sd_worker_ccx_" ++ integer_to_list(Idx)),
        #{id => WorkerId,
          start => {sd_worker, start_link, [Idx]},
          restart => permanent,
          shutdown => 5000,
          type => worker,
          modules => [sd_worker]}
    end, lists:zip(CcxList, lists:seq(0, length(CcxList) - 1))),
    
    {ok, {SupFlags, ChildSpecs}}.

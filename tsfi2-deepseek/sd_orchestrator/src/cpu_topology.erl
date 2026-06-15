-module(cpu_topology).

%% API
-export([discover_ccxs/0]).

%% @doc Automatically discovers CPU CCXs by grouping logical processors that share L3 caches.
-spec discover_ccxs() -> {ok, [string()]} | {error, term()}.
discover_ccxs() ->
    SysPath = "/sys/devices/system/cpu/",
    case file:list_dir(SysPath) of
        {ok, Files} ->
            CpuDirs = [F || F <- Files, is_cpu_dir(F)],
            CcxLists = lists:foldl(fun(CpuDir, Acc) ->
                SharedCpuFile = filename:join([SysPath, CpuDir, "cache", "index3", "shared_cpu_list"]),
                case file:read_file(SharedCpuFile) of
                    {ok, Bin} ->
                        CleanList = string:trim(binary_to_list(Bin)),
                        case lists:member(CleanList, Acc) of
                            true -> Acc;
                            false -> [CleanList | Acc]
                        end;
                    _ ->
                        %% Fallback if index3 is not readable
                        Acc
                end
            end, [], CpuDirs),
            case CcxLists of
                [] -> 
                    %% Fallback: treat the entire system as a single CCX
                    {ok, ["0-" ++ integer_to_list(erlang:system_info(logical_processors) - 1)]};
                _ ->
                    {ok, lists:sort(CcxLists)}
            end;
        {error, Reason} ->
            {error, Reason}
    end.

is_cpu_dir("cpu" ++ Rest) ->
    case catch list_to_integer(Rest) of
        I when is_integer(I) -> true;
        _ -> false
    end;
is_cpu_dir(_) ->
    false.

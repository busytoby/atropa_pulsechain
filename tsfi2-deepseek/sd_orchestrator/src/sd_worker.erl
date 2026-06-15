-module(sd_worker).
-behaviour(gen_server).

%% API
-export([start_link/1, generate/2]).

%% gen_server callbacks
-export([init/1, handle_call/3, handle_cast/2, handle_info/2, terminate/2, code_change/3]).

-define(SERVER, ?MODULE).

-record(state, {
    port :: port() | undefined,
    current_caller :: {pid(), reference()} | undefined,
    buffer = <<>> :: binary(),
    ccx_list = "" :: string()
}).

%%%===================================================================
%%% API
%%%===================================================================

start_link(CcxIdx) ->
    gen_server:start_link(?MODULE, [CcxIdx], []).

%% @doc Request frame-by-frame synthesis of an image using the native SD worker.
-spec generate(pid(), binary()) -> ok | {error, term()}.
generate(Pid, Payload) ->
    gen_server:call(Pid, {generate, Payload}, 60000).

%%%===================================================================
%%% gen_server callbacks
%%%===================================================================

init([CcxIdx]) ->
    process_flag(trap_exit, true),
    
    %% Automatically discover CCX lists
    CcxList = case cpu_topology:discover_ccxs() of
        {ok, Lists} when length(Lists) > CcxIdx ->
            lists:nth(CcxIdx + 1, Lists);
        {ok, [First | _]} ->
            First;
        _ ->
            "0"
    end,
    
    error_logger:info_msg("Starting sd_worker bound to CCX: ~s~n", [CcxList]),
    
    %% Resolve path to tsfi_sd_worker executable
    Executable = filename:join([code:priv_dir(sd_orchestrator), "..", "..", "bin", "tsfi_sd_worker"]),
    
    %% Wrap executable in taskset for CCX core pinning
    %% GGML thread count should match the number of cores in the CCX
    Args = ["-c", CcxList, Executable, "stdin", "tmp/vlm_sd_out.raw", "1", "dream", "4", "euler_a", "1.5"],
    
    Port = try
        open_port({spawn_executable, "/usr/bin/taskset"}, [
            {args, Args},
            binary,
            use_stdio,
            exit_status
        ])
    catch
        Error:Reason ->
            error_logger:error_msg("Failed to open native sd_worker port: ~p:~p~n", [Error, Reason]),
            undefined
    end,
    {ok, #state{port = Port, ccx_list = CcxList}}.

handle_call({generate, _Payload}, _From, #state{port = undefined} = State) ->
    {reply, {error, port_not_initialized}, State};
handle_call({generate, Payload}, From, #state{current_caller = undefined, port = Port} = State) ->
    port_command(Port, Payload),
    {noreply, State#state{current_caller = From, buffer = <<>>}};
handle_call({generate, _Payload}, _From, State) ->
    {reply, {error, busy}, State}.

handle_cast(_Msg, State) ->
    {noreply, State}.

handle_info({Port, {data, Data}}, #state{port = Port, current_caller = Caller, buffer = Buf} = State) ->
    NewBuf = <<Buf/binary, Data/binary>>,
    case binary:match(NewBuf, [<<"SUCCESS">>, <<"ERROR">>]) of
        {Pos, Len} ->
            Status = binary:part(NewBuf, Pos, Len),
            case Caller of
                undefined -> ok;
                _ -> gen_server:reply(Caller, {ok, Status})
            end,
            {noreply, State#state{current_caller = undefined, buffer = <<>>}};
        nomatch ->
            {noreply, State#state{buffer = NewBuf}}
    end;
handle_info({Port, {exit_status, Status}}, #state{port = Port, current_caller = Caller} = State) ->
    error_logger:warning_msg("Native sd_worker exited with status: ~p~n", [Status]),
    case Caller of
        undefined -> ok;
        _ -> gen_server:reply(Caller, {error, {worker_terminated, Status}})
    end,
    {stop, {worker_terminated, Status}, State#state{port = undefined, current_caller = undefined}};
handle_info({'EXIT', Port, Reason}, #state{port = Port, current_caller = Caller} = State) ->
    error_logger:error_msg("Native sd_worker port died: ~p~n", [Reason]),
    case Caller of
        undefined -> ok;
        _ -> gen_server:reply(Caller, {error, {port_died, Reason}})
    end,
    {stop, {port_died, Reason}, State#state{port = undefined, current_caller = undefined}};
handle_info(_Info, State) ->
    {noreply, State}.

terminate(_Reason, #state{port = Port}) ->
    case Port of
        undefined -> ok;
        _ -> 
            catch port_close(Port)
    end,
    ok.

code_change(_OldVsn, State, _Extra) ->
    {ok, State}.

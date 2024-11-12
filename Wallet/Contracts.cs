using Nethereum.Contracts;
using Nethereum.Contracts.Standards.ENS;
using Nethereum.Contracts.Standards.ENS.PublicResolver.ContractDefinition;
using Nethereum.Hex.HexTypes;
using Nethereum.RPC.Eth.DTOs;
using Nethereum.Generators;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http.Headers;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using System.Xml;
using Nethereum.ABI.ABIDeserialisation;
using Nethereum.Generators.Model;
using Nethereum.Web3;
using Nethereum.ABI.FunctionEncoding.Attributes;
using Dysnomia.Contracts;
using Nethereum.Contracts.ContractHandlers;
using Dysnomia.Contracts.VMREQ.ContractDefinition;
using Nethereum.Contracts.QueryHandlers.MultiCall;
using System.Xml.Linq;
using Nethereum.ABI.FunctionEncoding;

namespace Wallet
{
    public class Contracts
    {
        public Wallet Wallet;
        public Dictionary<string, Contract> Contract;
        (Event<LogEvent> Event, NewFilterInput Filter) Logs;
        public Dictionary<string, string> Aliases;
        static public string? Solc_bin;
        static public string? SolidityFolder;
        byte[] From = Encoding.Default.GetBytes("Contracts");

        public Contracts(Wallet wallet) {
            Wallet = wallet;
            Contract = new Dictionary<string, Contract>();
            Aliases = new Dictionary<string, string>();
        }

        public delegate void OutputCallback(byte[] From, byte[] Data, short Priority);

        public static void Init(string solc, string input) {
            if (File.Exists(solc))
                Solc_bin = solc;
            else throw (new Exception("No Such File"));

            if (Directory.Exists(input))
                SolidityFolder = input;
            else throw (new Exception("No Such Folder"));
        }

        public void AddAlias(string alias, string cxid) {
            Aliases.Add(alias, cxid);
        }

        /*
dysnomia/11_lau.sol
dysnomia/domain/assets/h2o.sol
dysnomia/domain/assets/vitus.sol
dysnomia/domain/dan/01_cho.sol
dysnomia/domain/dan/02c_systemaddresses.sol
dysnomia/domain/dan/03_qing.sol
dysnomia/domain/dan/04_war.sol
dysnomia/domain/map.sol
dysnomia/domain/sky/01_chan.sol
dysnomia/domain/sky/02_choa.sol
dysnomia/domain/sky/03_ring.sol
dysnomia/domain/soeng/01_qi.sol
dysnomia/domain/soeng/02_mai.sol
dysnomia/domain/soeng/03_xia.sol
dysnomia/domain/soeng/04_xie.sol
dysnomia/domain/soeng/05_zi.sol
dysnomia/domain/soeng/06_pang.sol
dysnomia/domain/soeng/07_gwat.sol
dysnomia/domain/tang/01_sei.sol
dysnomia/domain/tang/02_cheon.sol
dysnomia/domain/tang/03_meta.sol
dysnomia/domain/world.sol
dysnomia/domain/yue.sol
dysnomia/lib/encrypt.sol
dysnomia/lib/heckemeridians.sol
dysnomia/lib/multiownable.sol
dysnomia/lib/reactions_core.sol
dysnomia/lib/registry.sol
dysnomia/lib/stringlib.sol
dysnomia/lib/yai.sol.old
        */

        [Event("LogEvent")]
        public class LogEvent : IEventDTO {
            [Parameter("uint64", "Soul", 1, false)]
            public virtual ulong Soul { get; set; }
            [Parameter("uint64", "Aura", 2, false)]
            public virtual ulong Aura { get; set; }
            [Parameter("string", "LogLine", 3, false)]
            public virtual string? LogLine { get; set; }
        }

        [FunctionOutput]
        public class Shao : IFunctionOutputDTO {
            [Parameter("address", "Rod", 1)]
            public virtual string? Rod { get; set; }
            [Parameter("address", "Cone", 2)]
            public virtual string? Cone { get; set; }
            [Parameter("uint64", "Barn", 3)]
            public virtual ulong Barn { get; set; }
        }

        public async Task AddAliasWithABI(string alias, string cxid, string file) {
            (string ABI, string BIN) = Compile(file);
            Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
            Aliases[alias] = cxid;

        }

        public async Task Install(OutputCallback Output) {
            if(Output != null)
                Output(From, Encoding.Default.GetBytes("Deploying Everything"), 6);
            
            HexBigInteger latestBlock = await Wallet.w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();

            Event<LogEvent> YiShioLogEvent = Wallet.w3.Eth.GetEvent<LogEvent>();
            NewFilterInput _n = YiShioLogEvent.CreateFilterInput();
            _n.FromBlock = new BlockParameter(latestBlock);
            Logs = (YiShioLogEvent, _n);

            await _deploy(Output, "VMREQ", "dysnomia/00b_vmreq.sol");
            await _deploy(Output, "SHAFactory", "dysnomia/02c_shafactory.sol");
            await _deploy(Output, "SHIOFactory", "dysnomia/03c_shiofactory.sol");
            await _deploy(Output, "YI", "dysnomia/04_yi.sol", Aliases["SHAFactory"], Aliases["SHIOFactory"], Aliases["VMREQ"]);

            dynamic psi = await Execute(Contract[Aliases["YI"]], "Psi");
            AddAliasWithABI("YiShio", psi, "dysnomia/03_shio.sol");
            Output(From, Encoding.Default.GetBytes("YiShio" + " Deployed To: " + Aliases["YiShio"]), 6);
            await Execute(Contract[Aliases["YiShio"]], "Log", 5556, 0550, "Logging Test Successful");
            await Execute(Contract[Aliases["YiShio"]], "Log", 5556, 0550, "Logging Test Successful 2");
            
            /* this doesn't work
            Event<LogEvent> ysle = Contract[Aliases["YiShio"]].GetEvent<LogEvent>();
            NewFilterInput _n2 = ysle.CreateFilterInput();
            HexBigInteger _f = await ysle.CreateFilterAsync();
            //_n.FromBlock = new BlockParameter(latestBlock);
            //List<EventLog<LogEvent>> logs = await ysle.GetAllChangesAsync(_n2);
            List<EventLog<LogEvent>> logs = await ysle.GetFilterChangesAsync(_f);
            foreach(EventLog<LogEvent> _e in logs)
                Output(From, Encoding.Default.GetBytes("b" + _e.Log.BlockNumber + " s" + _e.Event.Soul + " a" + _e.Event.Aura + ": " + _e.Event.LogLine), 6);
            */ 

            Shao rho = await Execute(Contract[Aliases["YiShio"]], "Rho");
            _ = AddAliasWithABI("YiShioRod", rho.Rod, "dysnomia/02_sha.sol");
            Output(From, Encoding.Default.GetBytes("YiShioRod" + " Deployed To: " + Aliases["YiShioRod"]), 6);
            _ = AddAliasWithABI("YiShioCone", rho.Cone, "dysnomia/02_sha.sol");
            Output(From, Encoding.Default.GetBytes("YiShioCone" + " Deployed To: " + Aliases["YiShioCone"]), 6);

            await _deploy(Output, "ZHENG", "dysnomia/05_zheng.sol", Aliases["YI"]);
            await _deploy(Output, "ZHOU", "dysnomia/06_zhou.sol", Aliases["ZHENG"]);
            await _deploy(Output, "YAU", "dysnomia/07_yau.sol", Aliases["ZHOU"]);
            await _deploy(Output, "YANG", "dysnomia/08_yang.sol", Aliases["YAU"]);
            await _deploy(Output, "SIU", "dysnomia/09_siu.sol", Aliases["YANG"]);
            await _deploy(Output, "VOID", "dysnomia/10_void.sol", Aliases["SIU"]);
            await _deploy(Output, "ATTRIBUTE", "dysnomia/lib/attribute.sol", Aliases["VOID"]);
            await _deploy(Output, "LAUFactory", "dysnomia/11c_laufactory.sol", Aliases["VOID"]);
            await _deploy(Output, "STRINGLIB", "dysnomia/lib/stringlib.sol", Aliases["VOID"]);

            return;
        }

        public async Task<dynamic> Execute(Contract _c, string Function, params dynamic[] Args) {
            dynamic rx;
            switch (Function) {
                case "Rho":
                    rx = await _c.GetFunction("Rho").CallDeserializingToObjectAsync<Shao>();
                    break;
                default:
                    rx = await _c.GetFunction(Function).CallAsync<dynamic>(Args);
                    break;
        }
            
            /*
            Event _e = _c.GetEvent("Approval");
            NewFilterInput _f = _e.CreateFilterInput();
            Task<List<EventLog<dynamic>>> _l = _e.GetAllChangesAsync<dynamic>(_f);
            _l.Wait();
            List<EventLog<dynamic>> _rx2 = _l.Result;
            */
            return rx;
        }

        private async Task<Contract> DeployContract(string ABI,string BIN, params dynamic[] Args) {
            string txid = await Wallet.eth.DeployContract.SendRequestAsync(ABI, BIN, Wallet.Account.Address, new HexBigInteger(12000000), Args);
            TransactionReceipt Receipt = await Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
            string cxid = Receipt.ContractAddress;
            Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
            return Contract[cxid];
        }

        public async Task<string> _deploy(OutputCallback Output, string name, string file, params dynamic[] Args) {
            (string ABI, string BIN) = Compile(file);

            Contract _c = await DeployContract(ABI, BIN, Args);
            AddAlias(name, _c.Address);

            if(Output != null) {
                HexBigInteger latestBlock = await Wallet.w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();
                List<EventLog<LogEvent>> logs = await Logs.Event.GetAllChangesAsync(Logs.Filter);
                foreach(EventLog<LogEvent> _e in logs) 
                    Output(From, Encoding.Default.GetBytes("b" + _e.Log.BlockNumber + " s" + _e.Event.Soul + " a" + _e.Event.Aura + ": " + _e.Event.LogLine), 6);

                Output(From, Encoding.Default.GetBytes(name + " Deployed To: " + Aliases[name]), 6);
                Logs.Filter.FromBlock = new BlockParameter(latestBlock.ToUlong() + 1);
            }
            return Aliases[name];
        }

        public async Task<string> Deploy(string name, string file, params dynamic[] Args) {
            return await _deploy(null, name, file, Args);
        }

        //Function _f = _c.GetFunction("View");
        /*
        Task<ViewOutputDTO> _t2 = _f.CallDeserializingToObjectAsync<ViewOutputDTO>(Args);
        _t2.Wait();
        ViewOutputDTO rx = _t2.Result;
        */

        /*
        Dysnomia.Contracts.VMREQ.VmreqService _v = new Dysnomia.Contracts.VMREQ.VmreqService(Wallet.w3, _c.Address);
        Task<ViewOutputDTO> _t2 = _v.ViewQueryAsync();
        _t2.Wait();
        ViewOutputDTO rx = _t2.Result;

        dynamic _t = Execute(_c, "Random");
        _t = Execute(_c, "approve", _c.Address, "123");
        _t = Execute(_c, "totalSupply");
        _t = Execute(_c, "modExp", 123, 456, 789);
        */

        public static (string ABI, string BIN) Compile(string file) {
            string ABI = "", BIN = "";
            string diskfile = SolidityFolder + @"\" + file;
            Process _p = new Process();
            _p.StartInfo.FileName = Solc_bin;
            _p.StartInfo.Arguments = "--combined-json=bin,abi --optimize --optimize-runs=200 --base-path " + SolidityFolder + " --evm-version=shanghai " + diskfile;
            _p.StartInfo.RedirectStandardOutput = true;
            _p.StartInfo.RedirectStandardError = true;
            _p.StartInfo.UseShellExecute = false;
            _p.StartInfo.CreateNoWindow = true;
            _p.Start();
            string output = _p.StandardOutput.ReadToEnd();
            JsonDocument t = JsonDocument.Parse(output);
            JsonProperty t1 = t.RootElement.EnumerateObject().ToArray()[0];
            foreach (JsonProperty t2 in t1.Value.EnumerateObject().ToArray())
                if (t2.Name.Contains(file)) {
                    JsonProperty[] t3 = t2.Value.EnumerateObject().ToArray();
                    ABI = t3[0].Value.ToString();
                    BIN = t3[1].Value.ToString();
                    break;
                }
            _p.WaitForExit();
            return (ABI, BIN);
        }
    }
}

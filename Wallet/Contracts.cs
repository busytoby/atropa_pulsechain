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
using static System.Net.Mime.MediaTypeNames;

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

        [FunctionOutput]
        public class Bao : IFunctionOutputDTO {
            [Parameter("address", "Phi", 1)]
            public virtual string Phi { get; set; }
            [Parameter("address", "Mu", 2)]
            public virtual string Mu { get; set; }
            [Parameter("uint64", "Xi", 3)]
            public virtual ulong Xi { get; set; }
            [Parameter("uint64", "Pi", 4)]
            public virtual ulong Pi { get; set; }
            [Parameter("address", "Shio", 5)]
            public virtual string Shio { get; set; }
            [Parameter("uint64", "Ring", 6)]
            public virtual ulong Ring { get; set; }
            [Parameter("uint64", "Omicron", 7)]
            public virtual ulong Omicron { get; set; }
            [Parameter("uint64", "Omega", 8)]
            public virtual ulong Omega { get; set; }
        }

        public async Task AddAliasWithABI(string alias, string cxid, string file) {
            (string ABI, string BIN) = Compile(file);
            Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
            Aliases[alias] = cxid;

        }

        public async Task AddShioAliases(OutputCallback Output, string symbol, string address) {
            AddAliasWithABI(symbol + "Shio", address, "dysnomia/03_shio.sol");
            Output(From, Encoding.Default.GetBytes(symbol + "Shio" + " Deployed To: " + Aliases[symbol + "Shio"]), 6);
            _ = await Execute(Output, Contract[Aliases[symbol + "Shio"]], "Log", 5556, 1551, symbol + "Shio Logging Test Successful");

            Shao rho = await Execute(Output, Contract[Aliases[symbol + "Shio"]], "Rho");
            _ = AddAliasWithABI(symbol + "ShioRod", rho.Rod, "dysnomia/02_sha.sol");
            Output(From, Encoding.Default.GetBytes(symbol + "ShioRod" + " Deployed To: " + Aliases[symbol + "ShioRod"]), 6);
            _ = AddAliasWithABI(symbol + "ShioCone", rho.Cone, "dysnomia/02_sha.sol");
            Output(From, Encoding.Default.GetBytes(symbol + "ShioCone" + " Deployed To: " + Aliases[symbol + "ShioCone"]), 6);
        }

        public async Task DeployLau(OutputCallback Output, int walletnumber, string name, string symbol) {
            Wallet.SwitchAccount(walletnumber);
            (string ABI, string BIN) = Compile("dysnomia/11c_laufactory.sol");

            Contract _c = Wallet.eth.GetContract(ABI, Contract[Aliases["LAUFactory"]].Address);

            string lautx = await Execute(Output, _c, "New", name, symbol);
            TransactionReceipt receipt = await Wallet.w3.Eth.Transactions.GetTransactionReceipt.SendRequestAsync(lautx);
            while(receipt == null) {
                Thread.Sleep(1000);
                receipt = await Wallet.w3.Eth.Transactions.GetTransactionReceipt.SendRequestAsync(lautx);
            }

            string lau = receipt.Logs.First.Values<dynamic>().ToList()[0].Value;
            _ = AddAliasWithABI(symbol, lau, "dysnomia/11_lau.sol");
            string lautypeverifier = await Execute(Output, Contract[lau], "Type");
            if(lautypeverifier != "LAU") throw new Exception("LAU Type Verification Failed");

            Output(From, Encoding.Default.GetBytes(symbol +  " Deployed To: " + Aliases[symbol] + "From Wallet " + walletnumber), 6);
            Bao On = await Execute(Output, Contract[lau], "On");

            await AddShioAliases(Output, symbol, On.Shio);
            await GetLog(Output);
        }

        public async Task Install(OutputCallback Output) {
            Aliases = new Dictionary<string, string>();
            if(Output != null)
                Output(From, Encoding.Default.GetBytes("Deploying Everything"), 6);

            try {
                HexBigInteger Test = Wallet.EthGetBalance(Wallet.Account.Address);
                Nethereum.RPC.Eth.DTOs.TransactionReceipt rx = await Wallet.eth.GetEtherTransferService().TransferEtherAndWaitForReceiptAsync("0xC7cB8Eaead0ab55638d090c3a1DDE3E62E8e200b", 11, 200);
                Test = Wallet.EthGetBalance("0xC7cB8Eaead0ab55638d090c3a1DDE3E62E8e200b");

                HexBigInteger latestBlock = await Wallet.w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();

                Event<LogEvent> YiShioLogEvent = Wallet.w3.Eth.GetEvent<LogEvent>();
                NewFilterInput _n = YiShioLogEvent.CreateFilterInput();
                _n.FromBlock = new BlockParameter(latestBlock);
                Logs = (YiShioLogEvent, _n);

                await _deploy(Output, "VMREQ", "dysnomia/00b_vmreq.sol");
                await _deploy(Output, "SHAFactory", "dysnomia/02c_shafactory.sol");
                await _deploy(Output, "SHIOFactory", "dysnomia/03c_shiofactory.sol");
                await _deploy(Output, "YI", "dysnomia/04_yi.sol", Aliases["SHAFactory"], Aliases["SHIOFactory"], Aliases["VMREQ"]);

                dynamic psi = await Execute(Output, Contract[Aliases["YI"]], "Psi");
                await AddShioAliases(Output, "Yi", psi);

                await _deploy(Output, "ZHENG", "dysnomia/05_zheng.sol", Aliases["YI"]);
                await _deploy(Output, "ZHOU", "dysnomia/06_zhou.sol", Aliases["ZHENG"]);
                await _deploy(Output, "YAU", "dysnomia/07_yau.sol", Aliases["ZHOU"]);
                await _deploy(Output, "YANG", "dysnomia/08_yang.sol", Aliases["YAU"]);
                await _deploy(Output, "SIU", "dysnomia/09_siu.sol", Aliases["YANG"]);
                await _deploy(Output, "VOID", "dysnomia/10_void.sol", Aliases["SIU"]);
                await _deploy(Output, "ATTRIBUTE", "dysnomia/lib/attribute.sol", Aliases["VOID"]);
                await _deploy(Output, "LAUFactory", "dysnomia/11c_laufactory.sol", Aliases["VOID"]);
                await _deploy(Output, "STRINGLIB", "dysnomia/lib/stringlib.sol", Aliases["VOID"]);

                await DeployLau(Output, 0, "User Test", "UT0");
                await Execute(Output, Contract[Aliases["UT0"]], 0, "Username(string)", "Zero");
                await Execute(Output, Contract[Aliases["UT0"]], 0, "Chat", "Lau Test Chat Zero");
                await DeployLau(Output, 0, "User Test", "UT0_2");
                await Execute(Output, Contract[Aliases["UT0_2"]], 0, "Username(string)", "Zero Two");
                await Execute(Output, Contract[Aliases["UT0_2"]], 0, "Chat", "Lau Test Chat Zero Two");
                await DeployLau(Output, 1, "User Test", "UT1");
                await Execute(Output, Contract[Aliases["UT1"]], 1, "Username(string)", "One");
                await Execute(Output, Contract[Aliases["UT1"]], 1, "Chat", "Lau Test Chat One");
                await DeployLau(Output, 2, "User Test", "UT2");
                await Execute(Output, Contract[Aliases["UT2"]], 2, "Username(string)", "Two");
                await Execute(Output, Contract[Aliases["UT2"]], 2, "Chat", "Lau Test Chat Two");
                await DeployLau(Output, 3, "User Test", "UT3");
                await Execute(Output, Contract[Aliases["UT3"]], 3, "Username(string)", "Three");
                await Execute(Output, Contract[Aliases["UT3"]], 3, "Chat", "Lau Test Chat Three");
                await DeployLau(Output, 4, "User Test", "UT4");
                await Execute(Output, Contract[Aliases["UT4"]], 4, "Username(string)", "Four");
                await Execute(Output, Contract[Aliases["UT4"]], 4, "Chat", "Lau Test Chat Four");
                Wallet.SwitchAccount(0);

                await _deploy(Output, "react", "dysnomia/lib/reactions_core.sol", Aliases["VOID"]);
                await _deploy(Output, "CHO", "dysnomia/domain/dan/01_cho.sol", Aliases["VOID"]);
                //await _deploy(Output, "dan02csystemaddressscript", "dysnomia/domain/dan/02c_systemaddresses.sol", Aliases["CHO"]);


                await GetLog(Output);
            } catch (Exception _e) {
                int i = 99;
            }
            return;
        }

        /*
dysnomia/domain/assets/h2o.sol
dysnomia/domain/assets/vitus.sol
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
dysnomia/lib/registry.sol
dysnomia/lib/stringlib.sol
dysnomia/lib/yai.sol.old
*/

        public async Task<dynamic> Execute(OutputCallback Output, Contract _c, int Walletnumber, string Function, params dynamic[] Args) {
            Wallet.SwitchAccount(Walletnumber);
            // should possibly detect & reinstantiate contract for local wallet if it is not installed as so
            return await Execute(Output, _c, Function, Args);
        }

        public async Task<dynamic> Execute(OutputCallback Output, Contract _c, string Function, params dynamic[] Args) {
            dynamic rx = null;
            switch(Function) {
                case "Rho":
                    rx = await _c.GetFunction("Rho").CallDeserializingToObjectAsync<Shao>();
                    break;
                case "On":
                    rx = await _c.GetFunction("On").CallDeserializingToObjectAsync<Bao>();
                    break;
                default:
                    Nethereum.ABI.Model.FunctionABI _a = _c.ContractBuilder.ContractABI.Functions.FirstOrDefault(x => x.Signature.Contains(Function));
                    FunctionBuilder _fb = new FunctionBuilder(_c.Address, _a);
                    Function _f = new Function(_c, _fb);
                    if(_a.Signature.Contains(Function)) {
                        if(_a.Constant == true) {
                            rx = await _f.CallAsync<dynamic>(Args);
                            break;
                        } else {
                            HexBigInteger gas = await _f.EstimateGasAsync(Wallet.Account.Address, null, null, Args);
                            gas = new HexBigInteger((int)((double)gas.ToUlong() * 1.111));
                            rx = await _f.SendTransactionAsync(Wallet.Account.Address, gas, null, null, Args);
                            break;
                        }
                    } else throw new Exception("Error");
            }

            await GetLog(Output);
            return rx;
        }

        private async Task<Contract> DeployContract(string ABI,string BIN, params dynamic[] Args) {
            try {
                HexBigInteger gas = await Wallet.eth.DeployContract.EstimateGasAsync(ABI, BIN, Wallet.Account.Address, Args);
                gas = new HexBigInteger((int)((double)gas.ToUlong() * 1.111));

                string txid = await Wallet.eth.DeployContract.SendRequestAsync(ABI, BIN, Wallet.Account.Address, gas, Args);
                TransactionReceipt Receipt = await Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
                while(Receipt == null)
                    Receipt = await Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
                string cxid = Receipt.ContractAddress;
                Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
                return Contract[cxid];
            } catch (Exception _e) {
                int i = 99;
            }
            return null;
        }

        public async Task GetLog(OutputCallback Output) {
            if(Output != null) {
                HexBigInteger latestBlock = await Wallet.w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();
                List<EventLog<LogEvent>> logs = await Logs.Event.GetAllChangesAsync(Logs.Filter);
                foreach(EventLog<LogEvent> _e in logs)
                    Output(From, Encoding.Default.GetBytes("b" + _e.Log.BlockNumber + " s" + _e.Event.Soul + " a" + _e.Event.Aura + ": " + _e.Event.LogLine), 6);
                Logs.Filter.FromBlock = new BlockParameter(latestBlock.ToUlong() + 1);
            }
        }

        public async Task<string> _deploy(OutputCallback Output, string name, string file, params dynamic[] Args) {
            (string ABI, string BIN) = Compile(file);

            Contract _c = await DeployContract(ABI, BIN, Args);
            AddAlias(name, _c.Address);

            await GetLog(Output);
            Output(From, Encoding.Default.GetBytes(name + " Deployed To: " + Aliases[name]), 6);
            return Aliases[name];
        }

        public async Task<string> Deploy(string name, string file, params dynamic[] Args) {
            return await _deploy(null, name, file, Args);
        }

        public static (string ABI, string BIN) Compile(string file) {
            string ABI = "", BIN = "";
            string diskfile = SolidityFolder + @"\" + file;
            Process _p = new Process();
            _p.StartInfo.FileName = Solc_bin;
            _p.StartInfo.Arguments = "--combined-json=bin,abi --optimize --optimize-runs=200 --base-path " + SolidityFolder + " --evm-version=paris " + diskfile;
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

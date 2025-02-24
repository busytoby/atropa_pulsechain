﻿using Nethereum.Contracts;
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
using static Wallet.Events;
using Nethereum.RPC.Eth.Filters;
using Nethereum.RPC.TransactionManagers;

namespace Wallet
{
    public class Contracts
    {
        public Wallet Wallet;
        public Dictionary<string, string> ABIs;
        public Dictionary<string, Contract> Contract;
        internal static List<wEvent> Logs;
        static public string? Solc_bin;
        static public string? RootFolder;
        byte[] From = Encoding.Default.GetBytes("Contracts");

        static Contracts() {
            Logs = new List<wEvent>();
            Aliases.AddAlias("ZERO", "0x0000000000000000000000000000000000000000");
        }

        public Contracts(Wallet wallet) {
            Wallet = wallet;
            ABIs = new Dictionary<string, string>();
            Contract = new Dictionary<string, Contract>();

            if(!Aliases.Forward.ContainsKey(Wallet._base)) {
                string baseConfigFile = Path.Combine(RootFolder, Wallet._base);
                if(File.Exists(baseConfigFile)) {
                    foreach(string line in File.ReadAllLines(baseConfigFile)) {
                        string[] _alias = line.Split(0x0 + Wallet._base + 0x0);
                        Aliases.AddAlias(_alias[0], _alias[1]);
                    }
                }
            }
        }

        public delegate void OutputCallback(byte[] From, byte[] Data, short Priority);

        public static void Init(string solc, string input) {
            if (File.Exists(solc))
                Solc_bin = solc;
            else throw (new Exception("No Such File"));

            if (Directory.Exists(input))
                RootFolder = input;
            else throw (new Exception("No Such Folder"));
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
            ABIs[cxid] = ABI;
            Aliases.AddAlias(alias, cxid);
        }

        public async Task AddShioAliases(OutputCallback Output, string symbol, string address) {
            AddAliasWithABI(symbol + "Shio", address, "dysnomia/03_shio.sol");
            Output(From, Encoding.Default.GetBytes(symbol + "Shio" + " Deployed To: " + Aliases.Forward[symbol + "Shio"]), 6);
            _ = await Execute(Contract[Aliases.Forward[symbol + "Shio"]], "Log", 5556, 1551, symbol + "Shio Logging Test Successful");

            Shao rho = await Execute(Contract[Aliases.Forward[symbol + "Shio"]], "Rho");
            _ = AddAliasWithABI(symbol + "ShioRod", rho.Rod, "dysnomia/02_sha.sol");
            Output(From, Encoding.Default.GetBytes(symbol + "ShioRod" + " Deployed To: " + Aliases.Forward[symbol + "ShioRod"]), 6);
            _ = AddAliasWithABI(symbol + "ShioCone", rho.Cone, "dysnomia/02_sha.sol");
            Output(From, Encoding.Default.GetBytes(symbol + "ShioCone" + " Deployed To: " + Aliases.Forward[symbol + "ShioCone"]), 6);
        }

        public async Task DeployLau(OutputCallback Output, int walletnumber, string name, string symbol) {
            Wallet.SwitchAccount(walletnumber);
            (string ABI, string BIN) = Compile("dysnomia/11c_laufactory.sol");

            Contract _c = Wallet.eth.GetContract(ABI, Contract[Aliases.Forward["LAUFactory"]].Address);

            string lautx = await Execute(_c, "New", name, symbol);
            TransactionReceipt receipt = await Wallet.w3.Eth.Transactions.GetTransactionReceipt.SendRequestAsync(lautx);
            while(receipt == null) {
                Thread.Sleep(1000);
                receipt = await Wallet.w3.Eth.Transactions.GetTransactionReceipt.SendRequestAsync(lautx);
            }

            string lau = receipt.Logs.First.ToArray<dynamic>()[0].Value;
            _ = AddAliasWithABI(symbol, lau, "dysnomia/11_lau.sol");
            string lautypeverifier = await Execute(Contract[lau], "Type()");
            if(lautypeverifier != "LAU") throw new Exception("LAU Type Verification Failed");

            Output(From, Encoding.Default.GetBytes(symbol +  " Deployed To: " + Aliases.Forward[symbol] + "From Wallet " + walletnumber), 6);
            Bao On = await Execute(Contract[lau], "On");

            await AddShioAliases(Output, symbol, On.Shio);
            await ProcessLog(Output);
        }

        public async Task Install(OutputCallback Output, string g) {
            if(Output != null)
                Output(From, Encoding.Default.GetBytes("Deploying Everything"), 6);

            try {
                //HexBigInteger Test = Wallet.EthGetBalance(Wallet.Account.Address);
                //TransactionReceipt rx = await Wallet.eth.GetEtherTransferService().TransferEtherAndWaitForReceiptAsync("0xC7cB8Eaead0ab55638d090c3a1DDE3E62E8e200b", 111111, 200);
                //Test = Wallet.EthGetBalance("0xC7cB8Eaead0ab55638d090c3a1DDE3E62E8e200b");

                HexBigInteger latestBlock = await Wallet.w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();

                Event<TransferEvent> TransferEvent = Wallet.w3.Eth.GetEvent<TransferEvent>();
                NewFilterInput _n = TransferEvent.CreateFilterInput();
                _n.FromBlock = new BlockParameter(0);
                Logs.Add(new wEvent(TransferEvent, _n, "TransferEvent"));

                Event<LogEvent> YiShioLogEvent = Wallet.w3.Eth.GetEvent<LogEvent>();
                _n = YiShioLogEvent.CreateFilterInput();
                //_n.FromBlock = new BlockParameter(latestBlock);
                _n.FromBlock = new BlockParameter(0);
                Logs.Add(new wEvent(YiShioLogEvent, _n, "LogEvent"));

                /*
                await Deploy(Output, "VMREQ", "dysnomia/00b_vmreq.sol");
                await Deploy(Output, "SHAFactory", "dysnomia/02c_shafactory.sol");
                await Deploy(Output, "SHIOFactory", "dysnomia/03c_shiofactory.sol");
                await Deploy(Output, "YI", "dysnomia/04_yi.sol", Aliases.Forward["SHAFactory"], Aliases["SHIOFactory"], Aliases["VMREQ"]);

                dynamic psi = await Execute(Output, Contract[Aliases["YI"]], "Psi");
                await AddShioAliases(Output, "Yi", psi);

                await Deploy(Output, "ZHENG", "dysnomia/05_zheng.sol", Aliases["YI"]);
                await Deploy(Output, "ZHOU", "dysnomia/06_zhou.sol", Aliases["ZHENG"]);
                await Deploy(Output, "YAU", "dysnomia/07_yau.sol", Aliases["ZHOU"]);
                await Deploy(Output, "YANG", "dysnomia/08_yang.sol", Aliases["YAU"]);
                await Deploy(Output, "SIU", "dysnomia/09_siu.sol", Aliases["YANG"]);
                await Deploy(Output, "VOID", "dysnomia/10_void.sol", Aliases["SIU"]);
                await Deploy(Output, "ATTRIBUTE", "dysnomia/lib/attribute.sol", Aliases["VOID"]);
                await Deploy(Output, "LAUFactory", "dysnomia/11c_laufactory.sol", Aliases["VOID"]);
                await Deploy(Output, "STRINGLIB", "dysnomia/lib/stringlib.sol", Aliases["VOID"]);

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

                await Deploy(Output, "react", "dysnomia/lib/reactions_core.sol", Aliases["VOID"]);
                await Deploy(Output, "CHO", "dysnomia/domain/dan/01_cho.sol", Aliases["VOID"]);
                //await Deploy(Output, "dan02csystemaddressscript", "dysnomia/domain/dan/02c_systemaddresses.sol", Aliases["CHO"]);
                */

                await ProcessLog(Output);
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

        public async Task<dynamic> ExecuteWithAliases(string _a, int Walletnumber, string Function, params dynamic[] Args) {
            Wallet.SwitchAccount(Walletnumber);
            return await ExecuteWithAliases(_a, Function, Args);
        }

        public string ResolveAlias(string key) {
            string original = key;
            string alias = Aliases.Forward.FirstOrDefault(x => x.Key.ToLower().Contains(key.ToLower())).Key;
            while(alias != null) {
                key = Aliases.Forward[alias];
                if(Aliases.Forward.ContainsKey(key)) alias = Aliases.Forward[key];
                else alias = null;
                if(alias == key) break;
            }
            if(original != key || (key.StartsWith("0x") && key.Length == 42))
                return key;

            if(Aliases.Forward.ContainsKey(Wallet._base) && ABIs.ContainsKey(Aliases.Forward[Wallet._base])) {
                dynamic _t = Execute(Wallet.eth.GetContract(ABIs[Aliases.Forward[Wallet._base]], Aliases.Forward[Wallet._base]), "get", original);
                string result = Encoding.Default.GetString(_t.Result);
                if(result.Length > 0) return result;
            }
            return key;
        }

        public async Task<dynamic> ExecuteWithAliases(string _a, string Function, params dynamic[] Args) {
            _a = ResolveAlias(_a);
            Contract _c;
            if(ABIs.ContainsKey(_a))
                _c = Wallet.eth.GetContract(ABIs[_a], _a);
            else if(ABIs.ContainsKey(Wallet._base))
                _c = Wallet.eth.GetContract(ABIs[Wallet._base], _a);
            else if(ABIs.Count > 0) {
                _c = Wallet.eth.GetContract(ABIs.First().Value, _a);
            } else {
                AddAliasWithABI(Wallet._base, _a, "wallet/00_base.sol");
                _c = Contract[_a];
            }
            for(int i = 0; i < Args.Length; i++)
                Args[i] = ResolveAlias(Args[i]);

            if(Function == "get" && Args.Length == 1) return Args[0];
            return await Execute(_c, Function, Args);
        }

        public async Task<dynamic> Execute(Contract _c, int Walletnumber, string Function, params dynamic[] Args) {
            Wallet.SwitchAccount(Walletnumber);
            // should possibly detect & reinstantiate contract for local wallet if it is not installed as so
            return await Execute(_c, Function, Args);
        }

        public async Task<dynamic> Execute(Contract _c, string Function, params dynamic[] Args) {
            dynamic rx = null;
            try {
                switch(Function) {
                    case "Rho":
                        rx = await _c.GetFunction("Rho").CallDeserializingToObjectAsync<Shao>();
                        break;
                    case "On":
                        rx = await _c.GetFunction("On").CallDeserializingToObjectAsync<Bao>();
                        break;
                    default:
                        Nethereum.ABI.Model.FunctionABI _a = _c.ContractBuilder.ContractABI.Functions.FirstOrDefault(x => x.Signature.StartsWith(Function));
                        if(_a == null) _a = _c.ContractBuilder.ContractABI.Functions.FirstOrDefault(x => x.Signature.ToLower().StartsWith(Function.ToLower()));
                        if(_a != null) {
                            FunctionBuilder _fb = new FunctionBuilder(_c.Address, _a);
                            Function _f = new Function(_c, _fb);
                            if(_a.Constant == true) {
                                List<Nethereum.ABI.Model.Parameter> _p = _a.OutputParameters.ToList();
                                if(_p.Count == 1) {
                                    if(_p[0].Type == "string" || _p[0].Type == "address")
                                        rx = await _f.CallAsync<string>(Args);
                                    else
                                        rx = await _f.CallAsync<dynamic>(Args);
                                } else {
                                    int i = 99;
                                }
                                break;
                            } else {
                                HexBigInteger gas = new HexBigInteger(2000000);
                                if(Args.Length != _fb.FunctionABI.InputParameters.Length) throw new Exception("Wrong Number Of Arguments");
                                object[] callargs = new object[Args.Length];
                                for(int i = 0; i < Args.Length; i++) {
                                    if(_fb.FunctionABI.InputParameters[i].Type == "bytes")
                                        callargs[i] = Encoding.Default.GetBytes(Args[i]);
                                    else if(_fb.FunctionABI.InputParameters[i].Type == "address")
                                        callargs[i] = (string)Args[i];
                                    else if(_fb.FunctionABI.InputParameters[i].Type.Contains("int"))
                                        callargs[i] = System.Numerics.BigInteger.Parse(Args[i]);
                                    else
                                        callargs[i] = Args[i];
                                }
                                try {
                                    gas = await _f.EstimateGasAsync(Wallet.Account.Address, gas, null, callargs);
                                    gas = new HexBigInteger((int)((double)gas.ToUlong() * 1.1111));
                                } catch { }
                                rx = await _f.SendTransactionAsync(Wallet.Account.Address, gas, null, null, callargs);
                                TransactionReceipt tx = await this.Wallet.w3.TransactionManager.TransactionReceiptService.PollForReceiptAsync(rx);
                                if(tx.Status.Value == 1) {
                                    return "Success";
                                }
                                //rx = await _f.SendTransactionAndWaitForReceiptAsync(Wallet.Account.Address, gas, new HexBigInteger(0), new HexBigInteger(1), new HexBigInteger(70), new HexBigInteger(2), callargs);

                                /*
                string txid = await Wallet.eth.DeployContract.SendRequestAsync(ABI, BIN, Wallet.Account.Address, gas, Args);
                TransactionReceipt Receipt = await Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
                                */
                                break;
                            }
                        }
                        rx = null;
                        break;
                }
            } catch (Exception _e) {
                int i = 99;
            }
            return rx;
        }

        private async Task<Contract> DeployContract(string ABI,string BIN, params dynamic[] Args) {
            try {
                for(int i = 0; i < Args.Length; i++)
                    Args[i] = ResolveAlias(Args[i]);
                HexBigInteger gas = new HexBigInteger(2000000);
                try {
                    gas = await Wallet.eth.DeployContract.EstimateGasAsync(ABI, BIN, Wallet.Account.Address, Args);
                    gas = new HexBigInteger((int)((double)gas.ToUlong() * 1.1111));
                } catch { }
                string txid = await Wallet.eth.DeployContract.SendRequestAsync(ABI, BIN, Wallet.Account.Address, gas, Args);
                TransactionReceipt Receipt = await Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
                while(Receipt == null)
                    Receipt = await Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
                string cxid = Receipt.ContractAddress;
                Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
                ABIs.Add(cxid, ABI);
                return Contract[cxid];
            } catch (Exception _e) {
                int i = 99;
            }
            return null;
        }

        public async Task ProcessLog(OutputCallback Output) {
            foreach(wEvent w in Logs) {
                try {
                    if(Output != null) {
                        HexBigInteger latestBlock = await Wallet.w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();
                        if(w.Type == "TransferEvent") {
                            List<EventLog<TransferEvent>>  logs = await (w.WalletEvent as Event<TransferEvent>).GetAllChangesAsync(w.Filter);
                            foreach(EventLog<TransferEvent> _e in logs) {
                                string _from = _e.Event.From, _to = _e.Event.To, _address = _e.Log.Address;
                                if(Aliases.Reverse.ContainsKey(_from)) _from = Aliases.Reverse[_from];
                                if(Aliases.Reverse.ContainsKey(_to)) _to = Aliases.Reverse[_to];
                                if(Aliases.Reverse.ContainsKey(_address)) _address = Aliases.Reverse[_address];

                                if(_address.StartsWith("0x") && _address.Length == 42) {
                                    if(ABIs.ContainsKey(_address) && Contract.ContainsKey(_address)) {
                                        dynamic _symbol = await ExecuteWithAliases(_address, "symbol");
                                        int i = 99;
                                    } else {
                                        dynamic _symbol = await ExecuteWithAliases(_address, "symbol");
                                        if(_symbol.Length > 0 && _symbol != _address) {
                                            Aliases.AddAlias(_symbol, _address);
                                            _address = _symbol;
                                        }
                                    }
                                }

                                if(_from == Wallet.Account.Address || _to == Wallet.Account.Address)
                                    Output(From, Encoding.Default.GetBytes("b" + _e.Log.BlockNumber + " t" + _address + " f" + _from + " t" + _to + ": " + _e.Event.Value), 6);
                            }
                            w.Filter.FromBlock = new BlockParameter(latestBlock.ToUlong() + 1);
                        } else if(w.Type == "LogEvent") {
                            List<EventLog<LogEvent>>  logs = await (w.WalletEvent as Event<LogEvent>).GetAllChangesAsync(w.Filter);
                            foreach(EventLog<LogEvent> _e in logs)
                                Output(From, Encoding.Default.GetBytes("b" + _e.Log.BlockNumber + " s" + _e.Event.Soul + " a" + _e.Event.Aura + ": " + _e.Event.LogLine), 6);
                            w.Filter.FromBlock = new BlockParameter(latestBlock.ToUlong() + 1);
                        }
                    }
                } catch(Exception _e) {
                    // ignore
                }
            }
        }

        public async Task<string> Deploy(OutputCallback Output, string file, params dynamic[] Args) {
            string diskfile = RootFolder + @"\" + file;
            if(file.ToLower().EndsWith(".sol")) {
                (string ABI, string BIN) = Compile(file);
                for(int i = 0; i < Args.Length; i++)
                    Args[i] = ResolveAlias(Args[i]);
                Contract _c = await DeployContract(ABI, BIN, Args);
                string _cSymbol = await ExecuteWithAliases(_c.Address, "symbol()");
                if(_cSymbol == null) _cSymbol = Path.GetFileNameWithoutExtension(file);

                string address = _c.Address;
                if(Aliases.Forward.ContainsKey(Wallet._base) && ABIs.ContainsKey(Aliases.Forward[Wallet._base])) {
                    _c = Contract[Aliases.Forward[Wallet._base]];
                    Execute(_c, "set", _cSymbol, address);
                }
                Aliases.AddAlias(_cSymbol, address);

                Output(From, Encoding.Default.GetBytes(_cSymbol + " Deployed To: " + Aliases.Forward[_cSymbol]), 6);
                await ProcessLog(Output);
                return Aliases.Forward[_cSymbol];
            } else if(!file.ToLower().EndsWith(".dys"))
                file = file + ".dys";

            if(!File.Exists(diskfile)) diskfile = RootFolder + @"\scripts\" + file;
            if(!File.Exists(diskfile)) throw new Exception("File Not Found: " + file);
            foreach(string line in File.ReadAllLines(diskfile))
                Wallet.ProcessString(line);
            return null;
        }

        public static (string ABI, string BIN) Compile(string file) {
            string ABI = "", BIN = ""; 
            string _rootFolder = RootFolder;
            string diskfile = _rootFolder + @"\" + file;
            if(!File.Exists(diskfile)) _rootFolder += @"\solidity\";
            diskfile = _rootFolder + @"\" + file;
            if(!File.Exists(diskfile)) throw new Exception("File Not Found: " + file);
            Process _p = new Process();
            _p.StartInfo.FileName = Solc_bin;
            _p.StartInfo.Arguments = "--combined-json=bin,abi --optimize --optimize-runs=200 --base-path " + _rootFolder + " --evm-version=shanghai " + diskfile;
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

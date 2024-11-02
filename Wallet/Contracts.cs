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

namespace Wallet
{
    public class Contracts
    {
        public Wallet Wallet;
        public Dictionary<string, Contract> Contract;
        public Dictionary<string, string> Aliases;
        static public string? Solc_bin;
        static public string? SolidityFolder;

        public Contracts(Wallet wallet) {
            Wallet = wallet;
            Contract = new Dictionary<string, Contract>();
            Aliases = new Dictionary<string, string>();
        }

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

        public dynamic Execute(Contract _c, string Function, params dynamic[] Args) {
            Task<dynamic> _t2 = _c.GetFunction(Function).CallAsync<dynamic>(Args);
            _t2.Wait();
            dynamic rx = _t2.Result;
            /*
            Event _e = _c.GetEvent("Approval");
            NewFilterInput _f = _e.CreateFilterInput();
            Task<List<EventLog<dynamic>>> _l = _e.GetAllChangesAsync<dynamic>(_f);
            _l.Wait();
            List<EventLog<dynamic>> _rx2 = _l.Result;
            */
            return rx;
        }

        private Contract DeployContract(string ABI,string BIN, params dynamic[] Args) {
            Task<string> _t = Wallet.eth.DeployContract.SendRequestAsync(ABI, BIN, Wallet.Account.Address, new HexBigInteger(2000000), Args);
            _t.Wait();
            string txid = _t.Result;
            Task<TransactionReceipt> _t1 = Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
            _t1.Wait();
            TransactionReceipt Receipt = _t1.Result;
            string cxid = Receipt.ContractAddress;
            Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
            return Contract[cxid];
        }

        public string Deploy(string file, params dynamic[] Args) {
            (string ABI, string BIN) = Compile(file);

            Contract _c = DeployContract(ABI, BIN, Args);

            Function _f = _c.GetFunction("View");
            /*
            Task<ViewOutputDTO> _t2 = _f.CallDeserializingToObjectAsync<ViewOutputDTO>(Args);
            _t2.Wait();
            ViewOutputDTO rx = _t2.Result;
            */

            Dysnomia.Contracts.VMREQ.VmreqService _v = new Dysnomia.Contracts.VMREQ.VmreqService(Wallet.w3, _c.Address);
            Task<ViewOutputDTO> _t2 = _v.ViewQueryAsync();
            _t2.Wait();
            ViewOutputDTO rx = _t2.Result;

            dynamic _t = Execute(_c, "Random");
            _t = Execute(_c, "approve", _c.Address, "123");
            _t = Execute(_c, "totalSupply");
            _t = Execute(_c, "modExp", 123, 456, 789);

            return _c.Address;
        }

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

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

        /*
dysnomia/00b_vmreq.sol
dysnomia/02c_shafactory.sol
dysnomia/02_sha.sol
dysnomia/03c_shiofactory.sol
dysnomia/03_shio.sol
dysnomia/04_yi.sol
dysnomia/05_zheng.sol
dysnomia/06_zhou.sol
dysnomia/07_yau.sol
dysnomia/08_yang.sol
dysnomia/09_siu.sol
dysnomia/10_void.sol
dysnomia/11c_laufactory.sol
dysnomia/11_lau.sol
dysnomia/deploy.ts
dysnomia/domain
dysnomia/domain/assets
dysnomia/domain/assets/h2o.sol
dysnomia/domain/assets/interfaces
dysnomia/domain/assets/interfaces/h2ointerface.sol
dysnomia/domain/assets/interfaces/vitusinterface.sol
dysnomia/domain/assets/vitus.sol
dysnomia/domain/dan
dysnomia/domain/dan/01_cho.sol
dysnomia/domain/dan/02c_systemaddresses.sol
dysnomia/domain/dan/03_qing.sol
dysnomia/domain/dan/04_war.sol
dysnomia/domain/dan/interfaces
dysnomia/domain/dan/interfaces/01b_chointerface.sol
dysnomia/domain/dan/interfaces/03b_qinginterface.sol
dysnomia/domain/dan/interfaces/04b_warinterface.sol
dysnomia/domain/interfaces
dysnomia/domain/interfaces/mapinterface.sol
dysnomia/domain/interfaces/worldinterface.sol
dysnomia/domain/interfaces/yueinterface.sol
dysnomia/domain/map.sol
dysnomia/domain/old
dysnomia/domain/old/nym.sol
dysnomia/domain/sky
dysnomia/domain/sky/01_chan.sol
dysnomia/domain/sky/02_choa.sol
dysnomia/domain/sky/03_ring.sol
dysnomia/domain/sky/interfaces
dysnomia/domain/sky/interfaces/01b_chaninterface.sol
dysnomia/domain/sky/interfaces/02b_choainterface.sol
dysnomia/domain/sky/interfaces/03b_ringinterface.sol
dysnomia/domain/soeng
dysnomia/domain/soeng/01_qi.sol
dysnomia/domain/soeng/02_mai.sol
dysnomia/domain/soeng/03_xia.sol
dysnomia/domain/soeng/04_xie.sol
dysnomia/domain/soeng/05_zi.sol
dysnomia/domain/soeng/06_pang.sol
dysnomia/domain/soeng/07_gwat.sol
dysnomia/domain/soeng/interfaces
dysnomia/domain/soeng/interfaces/01b_qiinterface.sol
dysnomia/domain/soeng/interfaces/02b_maiinterface.sol
dysnomia/domain/soeng/interfaces/03b_xiainterface.sol
dysnomia/domain/soeng/interfaces/04b_xieinterface.sol
dysnomia/domain/soeng/interfaces/05b_ziinterface.sol
dysnomia/domain/soeng/interfaces/06b_panginterface.sol
dysnomia/domain/tang
dysnomia/domain/tang/01_sei.sol
dysnomia/domain/tang/02_cheon.sol
dysnomia/domain/tang/03_meta.sol
dysnomia/domain/tang/interfaces
dysnomia/domain/tang/interfaces/01b_seiinterface.sol
dysnomia/domain/tang/interfaces/02b_cheoninterface.sol
dysnomia/domain/tang/interfaces/03b_metainterface.sol
dysnomia/domain/world.sol
dysnomia/domain/yue.sol
dysnomia/etc
dysnomia/etc/DysnomiaSelfSnipe.sol
dysnomia/include
dysnomia/include/acronym.sol
dysnomia/include/bao.sol
dysnomia/include/fa.sol
dysnomia/include/user.sol
dysnomia/include/uservote.sol
dysnomia/interfaces
dysnomia/interfaces/01b_dysnomia.sol
dysnomia/interfaces/02b_shainterface.sol
dysnomia/interfaces/02d_shafactoryinterface.sol
dysnomia/interfaces/03b_shiointerface.sol
dysnomia/interfaces/03d_shiofactoryinterface.sol
dysnomia/interfaces/04b_yiinterface.sol
dysnomia/interfaces/05b_zhenginterface.sol
dysnomia/interfaces/06b_zhouinterface.sol
dysnomia/interfaces/07b_yauinterface.sol
dysnomia/interfaces/08b_yanginterface.sol
dysnomia/interfaces/09b_siuinterface.sol
dysnomia/interfaces/10b_voidinterface.sol
dysnomia/interfaces/11b_lauinterface.sol
dysnomia/interfaces/atropaMath.sol
dysnomia/interfaces/heckeinterface.sol
dysnomia/interfaces/libattribute.sol
dysnomia/interfaces/libcorereactions.sol
dysnomia/interfaces/libencrypt.sol
dysnomia/interfaces/libstrings.sol
dysnomia/interfaces/libyai.sol
dysnomia/interfaces/nyminterface.sol
dysnomia/lib
dysnomia/lib/attribute.sol
dysnomia/lib/encrypt.sol
dysnomia/lib/heckemeridians.sol
dysnomia/lib/multiownable.sol
dysnomia/lib/reactions_core.sol
dysnomia/lib/registry.sol
dysnomia/lib/stringlib.sol
dysnomia/lib/yai.sol.old
        */

        public void Install() {
            Deploy("VMREQ", "dysnomia/00b_vmreq.sol");
            Deploy("SHAFactory", "dysnomia/02c_shafactory.sol");
            Deploy("SHIOFactory", "dysnomia/03c_shiofactory.sol");
            Deploy("YI", "dysnomia/04_yi.sol", Aliases["SHAFactory"], Aliases["SHIOFactory"], Aliases["VMREQ"]);
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
            Task<string> _t = Wallet.eth.DeployContract.SendRequestAsync(ABI, BIN, Wallet.Account.Address, new HexBigInteger(12000000), Args);
            _t.Wait();
            string txid = _t.Result;
            Task<TransactionReceipt> _t1 = Wallet.eth.Transactions.GetTransactionReceipt.SendRequestAsync(txid);
            _t1.Wait();
            TransactionReceipt Receipt = _t1.Result;
            string cxid = Receipt.ContractAddress;
            Contract[cxid] = Wallet.eth.GetContract(ABI, cxid);
            return Contract[cxid];
        }

        public string Deploy(string name, string file, params dynamic[] Args) {
            (string ABI, string BIN) = Compile(file);

            Contract _c = DeployContract(ABI, BIN, Args);
            AddAlias(name, _c.Address);

            
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

            return Aliases[name];
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

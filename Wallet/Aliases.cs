using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Wallet {
    public class Aliases {
        static public Dictionary<string, string> Forward;
        static public Dictionary<string, string> Reverse;

        static Aliases() {
            Forward = new Dictionary<string, string>();
            Reverse = new Dictionary<string, string>();
        }

        static public void AddAlias(string alias, string cxid) {
            int offset = 0;
            string originalalias = alias;
            while(Forward.ContainsKey(alias)) alias = originalalias + "þ" + offset++;
            Forward.Add(alias, cxid);
            if(!Reverse.ContainsKey(cxid)) Reverse.Add(cxid, alias);
        }
    }
}

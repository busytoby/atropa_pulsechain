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
            if(Forward.ContainsKey(alias)) Forward.Remove(alias);
            Forward.Add(alias, cxid);
            if(!Reverse.ContainsKey(cxid)) Reverse.Add(cxid, alias);
        }
    }
}

using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;

namespace Dysnomia.Contracts.YAU.ContractDefinition
{
    public partial class Bao : BaoBase { }

    public class BaoBase 
    {
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
}

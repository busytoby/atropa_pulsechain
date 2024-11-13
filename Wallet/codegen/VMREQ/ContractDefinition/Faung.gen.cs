using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;

namespace Dysnomia.Contracts.VMREQ.ContractDefinition
{
    public partial class Faung : FaungBase { }

    public class FaungBase 
    {
        [Parameter("tuple", "Rod", 1)]
        public virtual Fa Rod { get; set; }
        [Parameter("tuple", "Cone", 2)]
        public virtual Fa Cone { get; set; }
        [Parameter("uint64", "Phi", 3)]
        public virtual ulong Phi { get; set; }
        [Parameter("uint64", "Eta", 4)]
        public virtual ulong Eta { get; set; }
        [Parameter("uint64", "Xi", 5)]
        public virtual ulong Xi { get; set; }
        [Parameter("uint64", "Sigma", 6)]
        public virtual ulong Sigma { get; set; }
        [Parameter("uint64", "Rho", 7)]
        public virtual ulong Rho { get; set; }
        [Parameter("uint64", "Upsilon", 8)]
        public virtual ulong Upsilon { get; set; }
        [Parameter("uint64", "Ohm", 9)]
        public virtual ulong Ohm { get; set; }
        [Parameter("uint64", "Pi", 10)]
        public virtual ulong Pi { get; set; }
        [Parameter("uint64", "Omicron", 11)]
        public virtual ulong Omicron { get; set; }
        [Parameter("uint64", "Omega", 12)]
        public virtual ulong Omega { get; set; }
        [Parameter("uint8", "Chi", 13)]
        public virtual byte Chi { get; set; }
    }
}

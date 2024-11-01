using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;

namespace Dysnomia.Contracts.VMREQ.ContractDefinition
{
    public partial class Fa : FaBase { }

    public class FaBase 
    {
        [Parameter("uint64", "Base", 1)]
        public virtual ulong Base { get; set; }
        [Parameter("uint64", "Secret", 2)]
        public virtual ulong Secret { get; set; }
        [Parameter("uint64", "Signal", 3)]
        public virtual ulong Signal { get; set; }
        [Parameter("uint64", "Channel", 4)]
        public virtual ulong Channel { get; set; }
        [Parameter("uint64", "Pole", 5)]
        public virtual ulong Pole { get; set; }
        [Parameter("uint64", "Identity", 6)]
        public virtual ulong Identity { get; set; }
        [Parameter("uint64", "Foundation", 7)]
        public virtual ulong Foundation { get; set; }
        [Parameter("uint64", "Element", 8)]
        public virtual ulong Element { get; set; }
        [Parameter("uint64", "Dynamo", 9)]
        public virtual ulong Dynamo { get; set; }
        [Parameter("uint64", "Manifold", 10)]
        public virtual ulong Manifold { get; set; }
        [Parameter("uint64", "Ring", 11)]
        public virtual ulong Ring { get; set; }
        [Parameter("uint64", "Barn", 12)]
        public virtual ulong Barn { get; set; }
        [Parameter("uint64", "Coordinate", 13)]
        public virtual ulong Coordinate { get; set; }
        [Parameter("uint64", "Tau", 14)]
        public virtual ulong Tau { get; set; }
        [Parameter("uint64", "Eta", 15)]
        public virtual ulong Eta { get; set; }
        [Parameter("uint64", "Kappa", 16)]
        public virtual ulong Kappa { get; set; }
        [Parameter("uint64", "Alpha", 17)]
        public virtual ulong Alpha { get; set; }
        [Parameter("uint8", "Nu", 18)]
        public virtual byte Nu { get; set; }
    }
}

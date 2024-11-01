using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;

namespace Dysnomia.Contracts.SHA.ContractDefinition
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
        [Parameter("uint64", "Contour", 5)]
        public virtual ulong Contour { get; set; }
        [Parameter("uint64", "Pole", 6)]
        public virtual ulong Pole { get; set; }
        [Parameter("uint64", "Identity", 7)]
        public virtual ulong Identity { get; set; }
        [Parameter("uint64", "Foundation", 8)]
        public virtual ulong Foundation { get; set; }
        [Parameter("uint64", "Element", 9)]
        public virtual ulong Element { get; set; }
        [Parameter("uint64", "Coordinate", 10)]
        public virtual ulong Coordinate { get; set; }
        [Parameter("uint64", "Charge", 11)]
        public virtual ulong Charge { get; set; }
        [Parameter("uint64", "Chin", 12)]
        public virtual ulong Chin { get; set; }
        [Parameter("uint64", "Monopole", 13)]
        public virtual ulong Monopole { get; set; }
    }
}

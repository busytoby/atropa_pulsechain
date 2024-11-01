using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;

namespace Dysnomia.Contracts.lib.COREREACTIONSLIB.ContractDefinition
{
    public partial class User : UserBase { }

    public class UserBase 
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("tuple", "On", 2)]
        public virtual Bao On { get; set; }
        [Parameter("string", "Username", 3)]
        public virtual string Username { get; set; }
        [Parameter("uint64", "Entropy", 4)]
        public virtual ulong Entropy { get; set; }
    }
}

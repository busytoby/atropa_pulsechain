using Nethereum.ABI.FunctionEncoding.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Wallet {
    public class Events {
        [Event("Approval")]
        public class ApprovalEvent : IEventDTO {
            [Parameter("address", "owner", 1, true)]
            public virtual string Owner { get; set; }
            [Parameter("address", "spender", 2, true)]
            public virtual string Spender { get; set; }
            [Parameter("uint256", "value", 3, false)]
            public virtual BigInteger Value { get; set; }
        }

        //public partial class TransferEventDTO : TransferEventDTOBase { }

        [Event("Transfer")]
        public class TransferEvent : IEventDTO {
            [Parameter("address", "from", 1, true)]
            public virtual string From { get; set; }
            [Parameter("address", "to", 2, true)]
            public virtual string To { get; set; }
            [Parameter("uint256", "value", 3, false)]
            public virtual BigInteger Value { get; set; }
        }

        [Event("LogEvent")]
        public class LogEvent : IEventDTO {
            [Parameter("uint64", "Soul", 1, false)]
            public virtual ulong Soul { get; set; }
            [Parameter("uint64", "Aura", 2, false)]
            public virtual ulong Aura { get; set; }
            [Parameter("string", "LogLine", 3, false)]
            public virtual string? LogLine { get; set; }
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Greed
    {
        public TcpClient Mu;
        public Fa Rho;
        public List<Living> Theta;

        public Greed(TcpClient Iota)
        {
            Mu = Iota;
            Rho = new Fa();
        }
    }
}

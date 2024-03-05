using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Lib
{
    public class DysnomiaNetworkStream
    {
        private TcpClient Mu;
        private NetworkStream Rho;
        public int ReadTimeout { get => Rho.ReadTimeout; set => Rho.ReadTimeout = value; }
        public bool DataAvailable { get { while (WaitingForProxy == true) Thread.Sleep(400); return Rho.DataAvailable; } }
        public bool WaitingForProxy = false;

        public DysnomiaNetworkStream(TcpClient Iota)
        {
            Mu = Iota;
            Rho = Mu.GetStream();
        }

        public void Write(byte[] data)
        {
            Rho.Write(data);
        }

        public int Read(Span<byte> data)
        {
            while (WaitingForProxy == true) Thread.Sleep(400);
            return Rho.Read(data);
        }

        public void Close()
        {
            Rho.Close();
        }
    }
}

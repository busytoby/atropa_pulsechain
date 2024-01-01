using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Numerics;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;
using Dysnomia.Domain.World;

namespace Dysnomia.Domain
{
    static public class Fi
    {
        static private TcpListener Mu;
        static public Tare Rho;
        static public ConcurrentDictionary<BigInteger, Greed> Psi;

        static Fi()
        {
            Rho = new Tare();
            Psi = new ConcurrentDictionary<BigInteger, Greed>();
        }

        static public void Listen(int port)
        {
            if (Mu != null) throw new Exception("Mu Non Null");
            Mu = new TcpListener(IPAddress.Any, port);
            Mu.Start();
            Mu.BeginAcceptTcpClient(Kappa, Mu);
        }

        static private void Kappa(IAsyncResult result)
        {
            TcpClient Beta = Mu.EndAcceptTcpClient(result);
            new Thread(() => Phi(Beta)).Start();
            Logging.Log("Fi", "Connected: " + ((IPEndPoint)Beta.Client.RemoteEndPoint).Address.ToString());
            Mu.BeginAcceptTcpClient(Kappa, Mu);
        }

        static private void Phi(TcpClient Beta)
        {
            BigInteger ClientId = Math.Random();
            while(Psi.ContainsKey(ClientId)) ClientId = Math.Random();

            if (!Psi.TryAdd(ClientId, new Greed(Beta))) throw new Exception("Failure Adding Client To Dictionary");

            NetworkStream Iota = Beta.GetStream();
            Iota.ReadTimeout = 100;
            byte[] bytes = new byte[32];
            Span<Byte> Omicron = new Span<Byte>(bytes);
            while (Beta.Connected)
            {
                if (Iota.DataAvailable)
                {
                    int size = Iota.Read(Omicron);
                    if (size > 0)
                    {
                        Tare.MSG M = new Tare.MSG(ClientId.ToByteArray(), Omicron.Slice(0, size).ToArray(), 1);
                        foreach (Tare.Gram G in Rho) G(M);
                    }
                }
                else
                    Thread.Sleep(400);

                try
                {
                    Iota.WriteByte(111);
                }
                catch (IOException E)
                {
                    Greed Lambda;
                    Psi.TryRemove(ClientId, out Lambda);
                    Logging.Log("Fi", "Disconnected: " + ((IPEndPoint)Beta.Client.RemoteEndPoint).Address.ToString());
                    break;
                }
            }
        }
    }
}

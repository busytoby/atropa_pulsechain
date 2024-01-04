#pragma warning disable CS0168

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Numerics;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;
using Dysnomia.Domain.bin;
using Dysnomia.Domain.World;
using Microsoft.VisualBasic;

namespace Dysnomia.Domain
{
    static public class Fi
    {
        static private TcpListener? Mu;
        static public Tare Rho;
        static public ConcurrentDictionary<BigInteger, Greed> Psi;

        static public String DLE = "\u0010\u0010\u0010\u0010";

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
            if (Mu == null) throw new Exception("Null Mu");
            TcpClient Beta = Mu.EndAcceptTcpClient(result);
            Socket S = Beta.Client;
            if (S.RemoteEndPoint == null) throw new Exception("Null EndPoint");
            IPEndPoint Remote = (IPEndPoint)S.RemoteEndPoint;

            foreach (KeyValuePair<BigInteger, Greed> P in Psi)
                if(P.Value.Cone && P.Value.Host == Remote.Address.ToString())
                {
                    Logging.Log("Fi", "Denied Already Connected: " + Remote.Address.ToString());
                    Beta.Close();
                    Mu.BeginAcceptTcpClient(Kappa, Mu);
                    return;
                }
            new Thread(() => Phi(Beta)).Start();
            Logging.Log("Fi", "Connected: " + Remote.Address.ToString());
            Mu.BeginAcceptTcpClient(Kappa, Mu);
        }

        static private bool ValidateMSG(Tare.MSG M)
        {
            throw new Exception("Not Implemented");
        }

        static private void XiHandshake(BigInteger ClientId)
        {
            Psi[ClientId].Theta.In.Enqueue(new Tare.MSG(Encoding.Default.GetBytes("Fi"), Encoding.Default.GetBytes("Xi"), ClientId.ToByteArray(), 1));
            Psi[ClientId].Theta.Out.Enqueue(new Tare.MSG(Encoding.Default.GetBytes("Fi"), Encoding.Default.GetBytes("Xi"), ClientId.ToByteArray(), 1));
        }

        static private void Phi(TcpClient Beta)
        {
            Greed? Client;
            BigInteger ClientId = Math.Random();
            Tare.MSG? Lambda;
            while(Psi.ContainsKey(ClientId)) ClientId = Math.Random();

            Client = new Greed(Beta);
            if (!Psi.TryAdd(ClientId, Client)) throw new Exception("Failure Adding Client To Dictionary");

            NetworkStream Iota = Beta.GetStream();
            Iota.ReadTimeout = 100;
            byte[] bytes = new byte[32];
            Span<Byte> Omicron = new Span<Byte>(bytes);

            XiHandshake(ClientId);

            // Drop Client If Handshake Incomplete
            try {
                while (Psi[ClientId].Psi == null) Thread.Sleep(1000);
            } catch(Exception E) {
                Psi.TryRemove(ClientId, out Client);
                return; 
            }

            while (Beta.Connected)
            {
                while (Client.Theta.In.Count > 0)
                {
                    if (!Client.Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                    String Subject = (Lambda.Subject == null) ? "" : Encoding.Default.GetString(Lambda.Subject);

                    if (Lambda.Data[0] == 0x07)
                    {
                        Logging.Log("Fi", "Handshake OK", 6);
                    }
                    else throw new Exception("Unknown OpCode");
                }

                while (Client.Theta.Out.Count > 0)
                {
                    if (!Client.Theta.Out.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                    if (Lambda != null && ValidateMSG(Lambda))
                    {
                        Iota.Write(Lambda.Data);
                        Iota.Write(Encoding.Default.GetBytes(Fi.DLE));
                    }
                }

                if (Iota.DataAvailable && !Psi[ClientId].Rho.Barn.IsZero)
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
                    Greed? Delta;
                    Psi.TryRemove(ClientId, out Delta);
                    if (Beta == null || Beta.Client == null || Beta.Client.RemoteEndPoint == null) throw new Exception("Null EndPoint");
                    Logging.Log("Fi", "Disconnected: " + ((IPEndPoint)Beta.Client.RemoteEndPoint).Address.ToString());
                    break;
                }
            }
        }

        public static Greed Connect(String Host, int Port)
        {
            Greed Beta = new Greed(Host, Port);
            return Beta;
        }
    }
}

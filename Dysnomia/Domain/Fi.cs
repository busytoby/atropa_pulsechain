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
using Dysnomia.Lib;
using Microsoft.VisualBasic;

namespace Dysnomia.Domain
{
    public class Fi : Daemon
    {
        new static public string Name = "Fi";
        new static public String Description = "Fi Daemon";

        private TcpListener? Mu;
        public Tare Rho;
        public ConcurrentDictionary<BigInteger, Greed> Psi;
        public Conjunction Nu;

        static public String DLE = "\u0010\u0010\u0010\u0010";

        public Fi() : base()
        {
            Rho = new Tare();
            Psi = new ConcurrentDictionary<BigInteger, Greed>();
            Nu = new Conjunction();
        }

        public void Listen(int port)
        {
            if (Mu != null) throw new Exception("Mu Non Null");
            Mu = new TcpListener(IPAddress.Any, port);
            Mu.Start();
            Mu.BeginAcceptTcpClient(Kappa, Mu);
        }

        private void Kappa(IAsyncResult result)
        {
            if (Mu == null) throw new Exception("Null Mu");
            TcpClient Beta = Mu.EndAcceptTcpClient(result);
            Socket S = Beta.Client;
            if (S.RemoteEndPoint == null) throw new Exception("Null EndPoint");
            IPEndPoint Remote = (IPEndPoint)S.RemoteEndPoint;

            if (IsAlreadyConnected(Remote))
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

        private bool IsAlreadyConnected(IPEndPoint Remote)
        {
            foreach (KeyValuePair<BigInteger, Greed> P in Psi)
                if (P.Value.Cone && P.Value.Host == Remote.Address.ToString())
                    return true;
            return false;
        }

        private bool ValidateTare(Tare M)
        {
            // not implemented
            return true;
        }

        private void XiHandshake(BigInteger ClientId)
        {
            if (!Psi.ContainsKey(ClientId)) throw new Exception("Unknown ClientId");
            Greed X = Psi[ClientId];
            if (X == null) throw new Exception("Null Greed");
            if (X.Theta == null) throw new Exception("Null Theta");
            X.Input("Fi", "Xi", ClientId.ToByteArray(), 1);
            X.Output("Fi", "Xi", ClientId.ToByteArray(), 1);
        }

        private void Push(byte[] From, byte[] Data)
        {
            try
            {
                Tare M = new Tare();
                M.Enqueue(From, Data, 1);
                foreach (Tare.Gram G in Rho.Subscribers) G(M);
            }
            catch (Exception e)
            {
                Logging.Log("Fi", "Error: " + e.Message, 7);
                Logging.Log("Fi", "Error: " + e.StackTrace, 7);
            }
        }

        private void Phi(TcpClient Beta)
        {
            Greed? Client;
            BigInteger ClientId = Math.Random();
            Tare? Lambda;
            while(Psi.ContainsKey(ClientId)) ClientId = Math.Random();

            Client = new Greed(Beta);
            if (!Psi.TryAdd(ClientId, Client)) throw new Exception("Failure Adding Client To Dictionary");

            Client.Rho[0].Rho = new DysnomiaNetworkStream(Beta);
#if DEBUG         
            Client.Rho[0].Rho.ReadTimeout = 300000;
#else
            Client.Rho[0].Rho.ReadTimeout = 8000;
#endif
            bool Proxying = false;
            byte[] bytes = new byte[256];
            Span<Byte> Omicron = new Span<Byte>(bytes);

            XiHandshake(ClientId);

            // Drop Client If Handshake Incomplete
            try {
                while (Psi[ClientId].Rho[0].Psi == null) Thread.Sleep(1000);
            } catch(Exception E) {
                //Logging.Log("Fi", E.Message, 7);
                //if(E.StackTrace != null) Logging.Log("Fi", E.StackTrace, 7);
                Psi.TryRemove(ClientId, out Client);
                return; 
            }

            while (Beta.Connected)
            {
                try
                {
                    if (Client.Theta == null) throw new Exception("Null Theta");
                    while (Client.Theta.In.Count > 0)
                    {
                        if (!Client.Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                        String From = Lambda.NextString();
                        String Subject = Lambda.NextString();
                        byte[] Data = Lambda.NextBytes();
                        byte[] Priority = Lambda.NextBytes();

                        if (Data[0] == 0x07)
                        {
                            Logging.Log("Fi", "Handshake OK", 6);
                            Push(ClientId.ToByteArray(), Data);
                        }

                        else throw new Exception("Unknown OpCode");
                    }

                    while (Client.Theta.Out.Count > 0)
                    {
                        if (!Client.Theta.Out.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                        if (Lambda != null && ValidateTare(Lambda))
                        {
                            short OpCode = Lambda.OpCode();
                            if (OpCode != 0x10 && OpCode != 0x11) throw new Exception("Unknown OpCode");
                            byte[] Timestamp = Lambda.NextBytes();
                            string From = Lambda.NextString();
                            string Subject = "";
                            if (OpCode == 0x11)
                                Subject = Lambda.NextString();
                            byte[] Data = Lambda.NextBytes();
                            byte[] Priority = Lambda.NextBytes();
                            Client.Rho[0].Rho.Write(Data);
                            Client.Rho[0].Rho.Write(Encoding.Default.GetBytes(Fi.DLE));
                        }
                    }

                    if (Client.Rho[0].Rho.DataAvailable && !Psi[ClientId].Rho[0].Mu.Barn.IsZero)
                    {
                        Thread.Sleep(200);
                        int size = Client.Rho[0].Rho.Read(Omicron);

                        int A, B;
                        for (int i = A = B = 0; i < size; i++)
                        {
                            if (i == A && Omicron[A] != 0x10) B = -1;
                            if (i >= A && Omicron.Slice(i, 4).SequenceEqual<Byte>(Encoding.Default.GetBytes(Fi.DLE)))
                            {
                                if (B == 0) A = i + 4;
                                B = i - A;
                            }
                            if (B <= 0) continue;

                            Span<Byte> Slice = Omicron.Slice(A, B);
                            if (Slice.Length == 1 && Slice[0] == 0x17)
                                Proxying = true;
                            else
                            {
                                if (!Proxying)
                                {
                                    BigInteger Alpha = new BigInteger(Slice);
                                    if (size > 0)
                                    {
                                        Push(ClientId.ToByteArray(), Slice.ToArray());
                                    }
                                } else
                                {
                                    Conjunction PC = Conjunction.Deserialize(Slice.ToArray());
                                    Conjunction Upsilon = new Conjunction();
                                    BigInteger PClientId = PC.Next();
                                    Upsilon.Enqueue(PClientId.ToByteArray());

                                    while (PC.Count > 1)
                                        Upsilon.Enqueue(PC.NextBytes());
                                    if (!Client.Rho.Indexes.ContainsKey(PClientId))
                                    {
                                        Client.Rho.Add(PClientId);
                                        Client.Rho[PClientId].Upsilon = Upsilon;
                                    }
                                    Fang Chi = Client.Rho[PClientId];

                                    BigInteger PCData = PC.Next();
                                    if (Client.Rho[PClientId].HandshakeState <= 0x07)
                                    {
                                        if (PC.Count == 0)
                                            Client.NextHandshake(ref PCData, ref Chi);
                                        else
                                            throw new Exception("Not Yet Implemented");
                                        Client.Rho[0].Rho.WaitingForProxy = true;
                                    }
                                    else
                                        Client.Procede(Slice, ref Chi);

                                    Proxying = false;
                                }
                            }

                            A = i + 4;
                            B = 0;
                            i += 3;
                        }
                        Omicron.Clear();
                    }
                    else
                        Thread.Sleep(400);

                    /*
                    try
                    {
                        Iota.WriteByte(0x07);
                        Thread.Sleep(1000);
                    }
                    catch (IOException E)
                    {
                        Greed? Delta;
                        Psi.TryRemove(ClientId, out Delta);
                        if (Beta == null || Beta.Client == null || Beta.Client.RemoteEndPoint == null) throw new Exception("Null EndPoint");
                        Logging.Log("Fi", "Disconnected: " + ((IPEndPoint)Beta.Client.RemoteEndPoint).Address.ToString());
                        break;
                    }
                    */
                }
                catch (Exception E)
                {
                    Logging.Log("Fi", "Error: " + E.Message, 7);
                    Logging.Log("Fi", "Error: " + E.StackTrace, 7);
                }
            }
        }
    }
}

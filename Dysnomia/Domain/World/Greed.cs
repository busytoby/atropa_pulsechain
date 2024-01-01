using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static Dysnomia.Domain.Tare;

namespace Dysnomia.Domain.World
{
    public class Greed
    {
        public TcpClient Mu;
        public Fa Rho;
        public Living Theta;
        private bool Cone = false;

        BigInteger PeerFoundation = 0;
        BigInteger PeerChannel = 0;
        BigInteger PeerDynamo = 0;

        public Greed(String Host, int Port)
        {
            Mu = new TcpClient();
            Rho = new Fa();
            Mu.Connect(new IPEndPoint(System.Net.Dns.GetHostAddresses(Host)[0], Port));
            Theta = new Living(Phi);
        }

        public Greed(TcpClient Iota)
        {
            Mu = Iota;
            Rho = new Fa();
            Theta = new Living(Phi);
            Cone = true;
        }

        private void Handshake(String Step, BigInteger Iota)
        {
            Logging.Log("Greed", String.Format("{0} {1} Handshake: {2}", Cone?"Cone":"Rod", Step, Iota, 1));
            Theta.Out.Enqueue(new Tare.MSG(Encoding.Default.GetBytes("Fi"), Encoding.Default.GetBytes(Step), Iota.ToByteArray(), 1));
        }

        void Phi()
        {
            byte[] bytes = new byte[1024];
            NetworkStream Iota = Mu.GetStream();
            Span<Byte> Omicron = new Span<Byte>(bytes);
            MSG Lambda;

            while (Mu.Connected)
            {
                try
                {
                    while (Theta.In.Count > 0)
                    {
                        if (!Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                        String Subject = Encoding.Default.GetString(Lambda.Subject);
                        if (Cone && Subject == "Xi")
                        {
                            BigInteger Delta = new BigInteger(Lambda.Data);
                            Rho.Tau = Rho.Avail(Delta);
                            Handshake("Tau", Rho.Tau);
                        }
                        else throw new Exception("Unknown Handshake Subject");
                    }

                    if (Iota.DataAvailable)
                    {
                        Thread.Sleep(200);
                        int size = Iota.Read(Omicron);
                        Queue<byte[]> Candidates = new Queue<byte[]>();

                        int A, B;
                        for (int i = A = B = 0; i < size; i++)
                        {
                            if (i == 0 && Omicron[0] != 0x10) B = -1;
                            if (i >= A && Omicron.Slice(i, 4).SequenceEqual<Byte>(Encoding.Default.GetBytes(Fi.DLE)))
                            {
                                if (B == 0) A = i + 4;
                                B = i - A;
                            }
                            if (B <= 0) continue;

                            Span<Byte> Token = Omicron.Slice(A, B);

                            if (Cone)
                            {
                                if (Rho.Pole.IsZero && PeerChannel.IsZero)
                                {
                                    BigInteger Alpha = new BigInteger(Token);
                                    Rho.Form(Alpha);
                                    Rho.Polarize();
                                    Handshake("Pole", Rho.Pole);
                                }
                                else if (Rho.Coordinate.IsZero)
                                {
                                    BigInteger Pole = new BigInteger(Token);
                                    Rho.Conjugate(ref Pole);
                                    Rho.Conify();
                                    Handshake("Foundation", Rho.Foundation);
                                    Handshake("Channel", Rho.Channel);
                                }
                                else if (Rho.Element.IsZero && PeerFoundation.IsZero)
                                {
                                    PeerFoundation = new BigInteger(Token);
                                }
                                else if (Rho.Element.IsZero && PeerChannel.IsZero)
                                {
                                    PeerChannel = new BigInteger(Token);
                                    Rho.Saturate(PeerFoundation, PeerChannel);
                                    Rho.Bond();
                                    Handshake("Dynamo", Rho.Dynamo);
                                }
                                else if (Rho.Ring.IsZero)
                                {
                                    PeerDynamo = new BigInteger(Token);
                                    Rho.Adduct(PeerDynamo);
                                    Rho.Open();
                                    Logging.Log("Greed", "Cone Handshake Complete: " + Rho.Barn, 2);
                                }
                                else
                                    throw new Exception("Not Implemented");
                            }
                            else
                            {
                                if (Rho.Alpha.IsZero)
                                {
                                    BigInteger Xi = new BigInteger(Token);
                                    Rho.Alpha = Rho.Avail(Xi);
                                    Handshake("Alpha", Rho.Alpha);
                                }
                                else if (Rho.Pole.IsZero && PeerChannel.IsZero)
                                {
                                    BigInteger Tau = new BigInteger(Token);
                                    Rho.Form(Tau);
                                    Rho.Polarize();
                                    Handshake("Pole", Rho.Pole);
                                }
                                else if (Rho.Coordinate.IsZero)
                                {
                                    BigInteger Pole = new BigInteger(Token);
                                    Rho.Conjugate(ref Pole);
                                }
                                else if (Rho.Element.IsZero && PeerFoundation.IsZero)
                                {
                                    PeerFoundation = new BigInteger(Token);
                                }
                                else if (Rho.Element.IsZero && PeerChannel.IsZero)
                                {
                                    PeerChannel = new BigInteger(Token);
                                    Rho.Saturate(PeerFoundation, PeerChannel);
                                    Rho.Bond();
                                    Handshake("Foundation", Rho.Foundation);
                                    Handshake("Channel", Rho.Channel);
                                    Handshake("Dynamo", Rho.Dynamo);
                                }
                                else if (Rho.Ring.IsZero)
                                {
                                    PeerDynamo = new BigInteger(Token);
                                    Rho.Adduct(PeerDynamo);
                                    Rho.Open();
                                    Logging.Log("Greed", "Rod Handshake Complete: " + Rho.Barn, 2);
                                }
                                else
                                    throw new Exception("Not Implemented");
                            }

                            A = i + 4;
                            B = 0;
                            i += 3;
                        }
                        Omicron.Clear();
                    }

                    while (Theta.Out.Count > 0)
                    {
                        if (!Theta.Out.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                        Iota.Write(Encoding.Default.GetBytes(Fi.DLE));
                        Iota.Write(Lambda.Data);
                        Iota.Write(Encoding.Default.GetBytes(Fi.DLE));
                    }
                } catch (Exception E) { }
            }
        }
    }
}

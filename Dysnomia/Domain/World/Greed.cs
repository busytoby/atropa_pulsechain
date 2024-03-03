#pragma warning disable CS0168

using Dysnomia.Domain.bin;
using Dysnomia.Lib;
using ExtensionMethods;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static Dysnomia.Lib.Tare;

namespace Dysnomia.Domain.World
{
    public class Greed : Daemon
    {
        new public static string Name = "Greed";
        new public static String Description = "Greed Daemon";

        public readonly String Host;
        public readonly int Port;
        public BigInteger ClientId;

        public TcpClient Mu;
        public Fa Rho;
        public Lib.Buffer? Psi;
        public Conjunction? Nu;
        public (BigInteger In, BigInteger Out) Eta;
        public Conjunction? Sigma;
        public Conjunction? Upsilon;
        public bool Cone = false;
        public bool TimedOut = false;
        public short HandshakeState = 0x00;

        BigInteger PeerFoundation = 0;
        BigInteger PeerChannel = 0;
        BigInteger PeerDynamo = 0;

        public Greed(BigInteger Proxy, Conjunction Chi)
        {
            Host = Controller.Fi.Psi[Proxy].Host;
            Port = Controller.Fi.Psi[Proxy].Port;
            Mu = Controller.Fi.Psi[Proxy].Mu;
            Rho = new Fa();
            Upsilon = Chi;
            Theta = new Living(Phi);
            Cone = true;
        }

        public Greed(String _Host, int _Port)
        {
            Host = _Host;
            Port = _Port;
            Mu = new TcpClient();
            Rho = new Fa();
            Theta = new Living(Phi);
        }

        public Greed(TcpClient Iota)
        {
            if (Iota.Client.RemoteEndPoint as IPEndPoint == null) throw new Exception("Null Client");
            Host = ((IPEndPoint)Iota.Client.RemoteEndPoint).Address.ToString();
            Port = ((IPEndPoint)Iota.Client.RemoteEndPoint).Port;
            Mu = Iota;
            Rho = new Fa();
            Theta = new Living(Phi);
            Cone = true;
        }

        /*
        public Lib.Buffer Encode(String Beta)
        {
            Logging.Log("Greed", "Encoding: " + Beta, 1);
            if (Psi == null) throw new Exception("Null Psi");
            Lib.Buffer A = new Lib.Buffer(Psi, Encoding.Default.GetBytes(Beta));
            Logging.Log("Greed", "Encoded Base64: " + Convert.ToBase64String(A.Bytes), 2);
            return A;
        }

        public Lib.Buffer Decode(Lib.Buffer Beta)
        {
            Logging.Log("Greed", "Decoding Base64: " + Convert.ToBase64String(Beta.Bytes), 1);
            if (Psi == null) throw new Exception("Null Psi");
            Lib.Buffer B = new Lib.Buffer(Psi, Beta.Bytes);
            Logging.Log("Greed", "Decoded: " + Encoding.Default.GetString(B.Bytes), 2);
            return B;
        }
        */

        public byte[] ProxyEncrypt(byte[] Data)
        {
            while (Upsilon.Count > 0)
            {
                BigInteger Proxy = Upsilon.Next();
                Controller.Fi.Psi[Proxy].Psi.Encode(Data, ref Controller.Fi.Psi[Proxy].Eta.Out);
                Data = Controller.Fi.Psi[Proxy].Psi.Bytes;
            }
            return Data;
        }

        public void Handshake(String Subject, byte[] Data)
        {
            if (Theta == null) throw new Exception("Null Theta");
            if (Subject != "Proxy" && Upsilon != null)
            {
                Handshake("Proxy", 0x16);
                Conjunction Omicron = new Conjunction();
                Data = ProxyEncrypt(Data);
            }

            Logging.Log("Greed", String.Format("{0} {1} Handshake: {2}", Cone ? "Cone" : "Rod", Subject, Encoding.Default.GetString(Data), 1), 2);
            Output("Fi", Subject, Data, 1);
        }

        public void Handshake(String Subject, BigInteger Data)
        {
            Handshake(Subject, Data.ToByteArray());
        }

        private void NextHandshake(ref BigInteger Beta, ref Fa Iota)
        {
            if (Theta == null) throw new Exception("Null Theta");
            if (Cone)
            {
                if(Iota.Tau.IsZero)
                {
                    Iota.Tau = Iota.Avail(Beta);
                    Handshake("Tau", Iota.Tau);
                    HandshakeState = 0x01;
                }
                else if (Iota.Pole.IsZero && PeerChannel.IsZero)
                {
                    Iota.Form(Beta);
                    Iota.Polarize();
                    Handshake("Pole", Iota.Pole);
                    HandshakeState = 0x02;
                }
                else if (Iota.Coordinate.IsZero)
                {
                    Iota.Conjugate(ref Beta);
                    Iota.Conify();
                    Handshake("Foundation", Iota.Foundation);
                    Handshake("Channel", Iota.Channel);
                    HandshakeState = 0x03;
                }
                else if (Iota.Element.IsZero && PeerFoundation.IsZero)
                {
                    PeerFoundation = Beta;
                    HandshakeState = 0x04;
                }
                else if (Iota.Element.IsZero && PeerChannel.IsZero)
                {
                    PeerChannel = Beta;
                    Iota.Saturate(PeerFoundation, PeerChannel);
                    Iota.Bond();
                    Handshake("Dynamo", Iota.Dynamo);
                    HandshakeState = 0x05;
                }
                else if (Iota.Barn.IsZero)
                {
                    PeerDynamo = Beta;
                    Iota.Adduct(PeerDynamo);
                    Iota.Open();
                    Logging.Log("Greed", "Cone Handshake Complete: " + Iota.Barn, 2);
                    Psi = new Lib.Buffer(Iota.Ring, Iota.Coordinate, Iota.Manifold, Iota.Barn, Iota.Element);
                    HandshakeState = 0x06;
                }
                else
                    throw new Exception("Not Implemented");
            }
            else
            {
                if (Iota.Alpha.IsZero)
                {
                    Iota.Alpha = Iota.Avail(Beta);
                    Handshake("Alpha", Iota.Alpha);
                    HandshakeState = 0x01;
                }
                else if (Iota.Pole.IsZero && PeerChannel.IsZero)
                {
                    Iota.Form(Beta);
                    Iota.Polarize();
                    Handshake("Pole", Iota.Pole);
                    HandshakeState = 0x02;
                }
                else if (Iota.Coordinate.IsZero)
                {
                    Iota.Conjugate(ref Beta);
                    HandshakeState = 0x03;
                }
                else if (Iota.Element.IsZero && PeerFoundation.IsZero)
                {
                    PeerFoundation = Beta;
                    HandshakeState = 0x04;
                }
                else if (Iota.Element.IsZero && PeerChannel.IsZero)
                {
                    PeerChannel = Beta;
                    Iota.Saturate(PeerFoundation, PeerChannel);
                    Iota.Bond();
                    Handshake("Foundation", Iota.Foundation);
                    Handshake("Channel", Iota.Channel);
                    Handshake("Dynamo", Iota.Dynamo);
                    HandshakeState = 0x05;
                }
                else if (Iota.Barn.IsZero)
                {
                    PeerDynamo = Beta;
                    Iota.Adduct(PeerDynamo);
                    Iota.Open();
                    Logging.Log("Greed", "Rod Handshake Complete: " + Iota.Barn, 2);
                    Psi = new Lib.Buffer(Iota.Ring, Iota.Coordinate, Iota.Manifold, Iota.Barn, Iota.Element);
                    Avail(Iota.Signal);
                    Form(Iota.Channel);
                    Psi.Pi();
                    Psi.Rho();
                    Eta = (Psi.Mu.Upsilon, Psi.Mu.Upsilon);
                    if (Nu != null) throw new Exception("Non Null Nu");
                    Nu = Controller.Fi.Psi[ClientId].Rho.OpenSerialization();
                    Sigma = new Conjunction();
                    HandshakeState = 0x06;
                }
                else
                    HandshakeState = (short)Beta;
            }
        }

        public void Avail(BigInteger Signal)
        {
            if (Psi == null) throw new Exception("Null Psi");
            if (Theta == null) throw new Exception("Null Theta");
            Psi.Alpha(Signal);
            Input("Fi", "ALPHA", new byte[] { 0x08 }, 1);
            Input("Fi", "ALPHA Signal", Signal.ToByteArray(), 1);
        }

        public void Form(BigInteger Channel)
        {
            if (Psi == null) throw new Exception("Null Psi");
            if (Theta == null) throw new Exception("Null Theta");
            Psi.Beta(Channel.ToByteArray(), false);
            if (Psi.Bytes == null) throw new Exception("Encoding Failure");
            Input("Fi", "BETA", new byte[] { 0x09 }, 1);
            Input("Fi", "BETA Psi", Psi.Bytes, 1);
        }

        public void Disconnect()
        {
            try {
                Greed? Beta;
                Controller.Fi.Psi.TryRemove(ClientId, out Beta);
                NetworkStream Iota = Mu.GetStream();
                try {
                    Iota.Write(Encoding.Default.GetBytes(String.Format("\r\n\r\n\r\n\r\n{0}\r\n", DysnomiaTime.Now)));
                    Iota.Write(Encoding.Default.GetBytes("Pre-Alpha Chatlog Milestone Test Client Available At https://github.com/busytoby/atropa_pulsechain\r\n"));
                    Iota.Close();
                } catch (Exception E) { }
            } catch (Exception E) { }
            try {
                Mu.Close();
            } catch (Exception E) { }
            Logging.Log("Greed", "Disconnected " + Host, 6);
        }

        private void Procede(Span<Byte> Iota)
        {
            switch(HandshakeState)
            {
                case 0x12:
                    String ChatString = Encoding.Default.GetString(Iota);
                    Psi.Gamma(ChatString);
                    Logging.Log("CHAT", ChatString, 12);
                    break;
                case 0x13:
                    Psi.Encode(Iota.ToArray(), ref Eta.In);
                    Psi.Gamma();
                    Logging.Log("ECHAT", Psi.ToString(), 12);
                    break;
                case 0x14:
                    Sigma.Enqueue(new byte[] { 0x14 });
                    Sigma.Enqueue(Iota.ToArray());
                    Logging.Log("QUERY", Encoding.Default.GetString(Iota), 1);
                    break;
                case 0x16:
                    throw new Exception("Not Yet Implemented");
                    break;
                default:
                    throw new Exception("Cannot Procede With Handshake State");
            }
            HandshakeState = 0x07;
        }

        protected override void Phi()
        {
            Thread.Sleep(10);
            if (Theta == null) throw new Exception("Null Theta");
            if (Upsilon == null)
            {
                if (!Mu.Connected && Theta.In.Count == 0 && Cone == false)
                    Mu.Connect(new IPEndPoint(Dns.GetHostAddresses(Host)[0], Port));
            }
            else
            {
                ClientId = Math.Random();
                Input("Fi", "Xi", ClientId.ToByteArray(), 1);
                Output("Fi", "Proxy", new byte[] { 0x16 }, 1);
                Output("Fi", "Xi", ProxyEncrypt(ClientId.ToByteArray()), 1);
            }
            byte[] bytes = new byte[256];
            NetworkStream Iota = Mu.GetStream();
            Stopwatch stopwatch = new Stopwatch();
            short Resets = 0;

            Span<Byte> Omicron = new Span<Byte>(bytes);
            Tare? Lambda;

            while (Mu.Connected)
            {
                try
                {
                    stopwatch.Start();
                    while (Theta.In.Count > 0)
                    {
                        string From, Subject;
                        byte[] Data, Priority;
                        if (!Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                        From = Lambda.NextString();
                        Subject = Lambda.NextString();
                        Data = Lambda.NextBytes();
                        Priority = Lambda.NextBytes();
                        if (Cone && Subject == "Xi")
                        {
                            BigInteger Delta = new BigInteger(Data);
                            ClientId = Delta;
                            NextHandshake(ref Delta, ref Rho);
                            stopwatch.Reset();
                        }
                        else if (Cone) throw new Exception("Cone Should No Longer Be Running In Greed");
                        else if (Subject == "ALPHA" && Data.Length == 1 && Data[0] == 0x08)
                        {
                            if (!Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                            From = Lambda.NextString();
                            Subject = Lambda.NextString();
                            Data = Lambda.NextBytes();
                            Priority = Lambda.NextBytes();
                            BigInteger Delta = new BigInteger(Data);
                            Handshake("Alpha", 0x08);
                            Handshake("Alpha", Delta);
                            if (Nu == null) throw new Exception("Null Nu");
                            Nu.Join(new byte[] { 0x08 }, Delta.ToByteArray());
                        }
                        else if (Subject == "BETA" && Data.Length == 1 && Data[0] == 0x09)
                        {
                            if (Psi == null) throw new Exception("Null Psi");
                            if (Psi.Bytes == null) throw new Exception("Null Psi Bytes");
                            if (!Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                            From = Lambda.NextString();
                            Subject = Lambda.NextString();
                            Data = Lambda.NextBytes();
                            Priority = Lambda.NextBytes();
                            Handshake("Beta", 0x09);
                            Handshake("Beta", Data);
                            if (Nu == null) throw new Exception("Null Nu");
                            Nu.Join(new byte[] { 0x09 }, Rho.Channel.ToByteArray());
                        }
                        else if (Subject == "SAY" && Data.Length == 1 && Data[0] == 0x12)
                        {
                            if (Psi == null) throw new Exception("Null Psi");
                            if (Psi.Bytes == null) throw new Exception("Null Psi Bytes");
                            if (!Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                            From = Lambda.NextString();
                            Subject = Lambda.NextString();
                            Data = Lambda.NextBytes();
                            Priority = Lambda.NextBytes();
                            Handshake("Say", 0x12);
                            Handshake("Say", Data);
                        }
                        else if (Subject == "ESAY" && Data.Length == 1 && Data[0] == 0x13)
                        {
                            if (Psi == null) throw new Exception("Null Psi");
                            if (Psi.Bytes == null) throw new Exception("Null Psi Bytes");
                            if (!Theta.In.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                            From = Lambda.NextString();
                            Subject = Lambda.NextString();
                            Psi.Encode(Lambda.NextBytes(), ref Eta.Out);
                            Data = Psi.Bytes;
                            Priority = Lambda.NextBytes();
                            Handshake("Esay", 0x13);
                            Handshake("Esay", Data);
                        }
                        else throw new Exception("Unknown Handshake Subject");
                    }

                    while (Theta.Out.Count > 0)
                    {
                        if (!Theta.Out.TryDequeue(out Lambda)) throw new Exception("Cannot Dequeue");
                        short OpCode = Lambda.OpCode();
                        if (OpCode != 0x10 && OpCode != 0x11) throw new Exception("Unknown OpCode");
                        byte[] Timestamp = Lambda.NextBytes();
                        string From = Lambda.NextString();
                        string Subject = "";
                        if(OpCode == 0x11)
                            Subject = Lambda.NextString();
                        byte[] Data = Lambda.NextBytes();
                        byte[] Priority = Lambda.NextBytes();
                        Iota.Write(Data);
                        Iota.Write(Encoding.Default.GetBytes(Fi.DLE));
                    }

                    Thread.Sleep(200);
                    if (Iota.DataAvailable)
                    {
                        Thread.Sleep(200);
                        int size = Iota.Read(Omicron);

                        int A, B;
                        for (int i = A = B = 0; i < size; i++)
                        {
                            //if (i == A && Omicron[A] == 0x07) { A++; continue; }
                            if (i == A && Omicron[A] != 0x10) B = -1;
                            if (i >= A && Omicron.Slice(i, 4).SequenceEqual<Byte>(Encoding.Default.GetBytes(Fi.DLE)))
                            {
                                if (B == 0) A = i + 4;
                                B = i - A;
                            }
                            if (B <= 0) continue;

                            Span<Byte> Slice = Omicron.Slice(A, B);
                            if (HandshakeState <= 0x07)
                            {
                                BigInteger Alpha = new BigInteger(Slice);
                                NextHandshake(ref Alpha, ref Rho);
                            }
                            else
                                Procede(Slice);
                            stopwatch.Reset();

                            A = i + 4;
                            B = 0;
                            i += 3;
                        }
                        Omicron.Clear();
                    }

                    if (Cone && HandshakeState == 0x06)
                    {
                        Input("Fi", "OK", new byte[] { 0x07 }, 1);
                        return;
                    }
                    stopwatch.Stop();
                    if (Rho.Barn.IsZero && stopwatch.Elapsed.TotalSeconds > 3000)
                        if (++Resets > 2) throw new Exception("Handshake Timeout");
                        else
                        {
                            Logging.Log("Greed", "Handshake Timeout, Sending Reset", 6);
                            Output("Fi", "Reset", new byte[] { 0x06 }, 1);
                            Rho = new Fa();
                            stopwatch.Reset();
                        }
                    stopwatch.Start();
                } catch (Exception E) {
                    //Logging.Log("Fi", E.Message, 7);
                    //if (E.StackTrace != null) Logging.Log("Fi", E.StackTrace, 7);
                    Disconnect(); 
                    return; 
                }
            }
        }
    }
}

using Dysnomia.Domain.bin;
using Dysnomia.Domain.World;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Numerics;
using System.Security.Policy;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Oracle : Daemon
    {
        new public static string Name = "Oracle";
        new public static String Description = "Oracle Daemon";
        public Faung Mu;
        public Object Tau;

        public Oracle()
        {
            Logging.Log("Oracle", "New Oracle");
            Mu = new Faung();
            Tau = new Object();
            Theta = new Living(Phi);
            Reset();
        }

        public void Fi(Tare.MSG A)
        {
            BigInteger ClientId = new BigInteger(A.From);
            if (!Dysnomia.Beta.Fi.Psi.ContainsKey(ClientId)) throw new Exception("Unknown ClientId");
            Enqueue(A.Data);
            Enqueue(A.From);
        }

        public void ProcessString(String A)
        {
            Logging.Log("Oracle", "ProcessString: " + A, 3);
            Enqueue(new byte[] { 0x02 });
            Enqueue(Encoding.Default.GetBytes(A));
        }

        /*
        public Buffer Encode(String Beta)        
        {
            Logging.Log("Oracle", "Encoding: " + Beta, 1);
            Buffer A = new Buffer(Mu, Encoding.Default.GetBytes(Beta));
            Logging.Log("Oracle", "Encoded Base64: " + Convert.ToBase64String(A.Bytes), 2);
            return A;
        }

        public Buffer Decode(Buffer Beta)
        {
            Logging.Log("Oracle", "Decoding Base64: " + Convert.ToBase64String(Beta.Bytes), 1);
            Buffer B = new Buffer(Mu, Beta.Bytes);
            Logging.Log("Oracle", "Decoded: " + Encoding.Default.GetString(B.Bytes), 2);
            return B;
        }
        */

        public void Reset()
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            Logging.Log("Oracle", "Reset", 5);
            Alpha(ref Mu, Mu.Rod.Signal);
        }

        private void Alpha(ref Faung _Mu, BigInteger Omicron)
        {
            if (_Mu.Rod == null) throw new Exception("Null Rod");
            if (_Mu.Cone == null) throw new Exception("Null Cone");
            Logging.Log("Oracle", "Alpha: " + Omicron.ToString(), 4);
            lock (Tau)
            {
                if (Omicron == 0) throw new Exception("Omicron Zero");

                _Mu.Charge(Omicron);
                if (_Mu.Sigma < 4)
                    throw new Exception("Sigma < 4");
                _Mu.Induce();
                _Mu.Torque();
                _Mu.Amplify();
                _Mu.Sustain();
                _Mu.React();

                Enqueue(new byte[] { 0x00 });
                Enqueue(Omicron.ToByteArray());
                Enqueue(_Mu.Sigma.ToByteArray());
                Enqueue(_Mu.Cone.Channel.ToByteArray());
                Enqueue(_Mu.Rod.Channel.ToByteArray());
                Enqueue(_Mu.Rod.Kappa.ToByteArray());
            }
        }

        public BigInteger Op(String Beta)
        {
            byte[] Iota = Encoding.Unicode.GetBytes(Beta);
            BigInteger Omicron = new BigInteger(Iota);
            return Omicron;
        }

        public void Beta(ref Faung _Mu, BigInteger Omicron)
        {
            if (_Mu.Rod == null) throw new Exception("Null Rod");
            if (_Mu.Cone == null) throw new Exception("Null Cone");
            Logging.Log("Oracle", "Beta: " + Omicron.ToString(), 4);
            lock (Tau)
            {
                if (_Mu.Omicron == 0) throw new Exception("Mu Omicron Zero");
                if (Omicron == 0) throw new Exception("Iota Zero");

                BigInteger Lambda = _Mu.Rod.Torque(Omicron);
                Lambda = _Mu.Rod.Amplify(Lambda);
                Lambda = _Mu.Rod.Sustain(Lambda);
                _Mu.Rod.React(Lambda, _Mu.Cone.Dynamo);
                _Mu.Cone.React(Lambda, _Mu.Rod.Dynamo);

                Enqueue(new byte[] { 0x01 });
                Enqueue(Omicron.ToByteArray());
                Enqueue(_Mu.Cone.Dynamo.ToByteArray());
                Enqueue(_Mu.Rod.Dynamo.ToByteArray());
                Enqueue(_Mu.Rod.Eta.ToByteArray());
                Enqueue(_Mu.Cone.Eta.ToByteArray());
            }
        }

        protected override void Phi()
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            int _sleep = 20;
            byte[]? OpCode;

            BigInteger Iota, Omicron;
            byte[]? Lambda;
            String Xi;
            Tare.MSG? Pi;

            while (true)
            {
                lock (Tau)
                {
                    if (Count > 0) _sleep = 20;
                    TryDequeue(out OpCode);
                    while (Count > 0)
                    {
                        if (OpCode == null) TryDequeue(out OpCode);
                        if (OpCode == null || OpCode.Length != 1) throw new Exception("Bad OpCode");
                        switch(OpCode[0])
                        {
                            case 0x00:
                                Iota = Next();
                                Omicron = Next();
                                if (Mu.Cone.Mu(Mu.Cone.Barn, Iota, Mu.Cone.Ring) != Omicron) throw new Exception("Invalid Charge");

                                Iota = Next();
                                if (Iota != Mu.Cone.Channel) throw new Exception("Bad Cone Channel");

                                Iota = Next();
                                if (Iota != Mu.Rod.Channel) throw new Exception("Bad Rod Channel");

                                Iota = Next();
                                Omicron = Mu.Rod.Induce(Omicron);
                                Omicron = Mu.Cone.Torque(Omicron);
                                Omicron = Mu.Cone.Amplify(Omicron);
                                Omicron = Mu.Cone.Sustain(Omicron);
                                if (Mu.Rod.Mu(Omicron, Mu.Cone.Channel, Mu.Rod.Channel) != Iota) throw new Exception("Invalid Reaction");
                                Logging.Log("Oracle", "Alpha Operational: " + Omicron.ToString(), 3);
                                break;
                            case 0x01:
                                Iota = Next();
                                Omicron = Next();
                                if (Omicron != Mu.Cone.Dynamo) throw new Exception("Bad Cone Dynamo");
                                Omicron = Next();
                                if (Omicron != Mu.Rod.Dynamo) throw new Exception("Bad Rod Dynamo");

                                Iota = Mu.Rod.Torque(Iota);
                                Iota = Mu.Rod.Amplify(Iota);
                                Iota = Mu.Rod.Sustain(Iota);

                                Omicron = Next();
                                if (Mu.Rod.Mu(Iota, Mu.Rod.Channel, Mu.Cone.Dynamo) != Omicron) throw new Exception("Invalid Rod Eta");

                                Omicron = Next();
                                if (Mu.Cone.Mu(Iota, Mu.Cone.Channel, Mu.Rod.Dynamo) != Omicron) throw new Exception("Invalid Cone Eta");
                                Logging.Log("Oracle", "Beta Operational: " + Mu.Sigma.ToString(), 3);
                                break;
                            case 0x02:
                                Lambda = NextBytes();
                                if (Lambda == null) throw new Exception("Heap Corrupted");
                                Xi = Encoding.Default.GetString(Lambda);
                                Logging.Log("Oracle", "EXEC: " + Xi, 3);
                                Command command = new Command(Xi);
                                if (command.Theta != null)
                                {
                                    while (command.Theta.Alive()) Thread.Sleep(100);
                                    while (command.Theta.In.Count > 0)
                                        if (command.Theta.In.TryDequeue(out Pi))
                                            Enqueue(Pi.Data);
                                    while (command.Theta.Out.Count > 0)
                                        if (command.Theta.Out.TryDequeue(out Pi))
                                            Logging.Log(Pi);
                                }
                                break;
                            case 0x03:
                                while (Count < 1) Thread.Sleep(100);
                                Lambda = NextBytes();
                                if (Lambda == null) throw new Exception("Heap Corrupted");
                                if (Lambda[0] == 0x00) Reset();
                                else Alpha(ref Mu, new BigInteger(Lambda));
                                break;
                            case 0x04:
                                while (Count < 1) Thread.Sleep(100);
                                Lambda = NextBytes();
                                if (Lambda == null) throw new Exception("Heap Corrupted");
                                Beta(ref Mu, new BigInteger(Lambda));
                                break;
                            case 0x05:
                                while (Count < 2) Thread.Sleep(100);
                                Lambda = NextBytes();
                                if (Lambda == null) throw new Exception("Heap Corrupted");
                                String Connect_Host = Encoding.Default.GetString(Lambda);
                                Lambda = NextBytes();
                                if (Lambda == null) throw new Exception("Heap Corrupted");
                                int Connect_Port = BitConverter.ToInt16(Lambda, 0);
                                Greed Chi = Dysnomia.Beta.Fi.Connect(Connect_Host, Connect_Port);
                                if (!Chi.ClientId.IsZero) throw new Exception("Client ID Non-Zero");
                                Chi.ClientId = Math.Random();
                                Dysnomia.Beta.Fi.Psi.TryAdd(Chi.ClientId, Chi);
                                break;
                            case 0x06:
                                throw new Exception("Handshake Correction Not Yet Implemented");
                                break;
                            case 0x08:
                                while(Count < 3) Thread.Sleep(100);
                                BigInteger ClientId = Next();
                                BigInteger AlphaCode = Next();
                                BigInteger ClientIdCheck = Next();
                                if (ClientId != ClientIdCheck) throw new Exception("OpCode 0x08 ClientId Error");
                                if (!Dysnomia.Beta.Fi.Psi.ContainsKey(ClientId)) throw new Exception("OpCode 0x08 Unknown ClientId");
                                Greed A = Dysnomia.Beta.Fi.Psi[ClientId];
                                if (A.Psi == null) throw new Exception("Null Psi For ClientId: " + ClientId);
                                A.Psi.Alpha(AlphaCode);
                                break;
                            default:
                                throw new Exception("Not Implemented");
                        }

                        if (Count > 0) TryDequeue(out OpCode);
                    }
                    if (Count == 0) _sleep *= 2;
                }
                if (_sleep > 1551) _sleep = 1551;
                Thread.Sleep(_sleep);
            }
        }
    }
}

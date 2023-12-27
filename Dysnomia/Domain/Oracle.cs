using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Numerics;
using System.Security.Policy;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Oracle : ConcurrentQueue<byte[]>
    {
        public Faung Mu;
        public Mutex Tau = new Mutex(false);
        public Living Theta;

        public Oracle()
        {
            Mu = new Faung();
            Theta = new Living(Phi);
            Reset();
        }

        public Buffer Encode(String Beta)
        {            
            return new Buffer(Mu, Encoding.ASCII.GetBytes(Beta));
        }

        public Buffer Decode(Buffer Beta)
        {
            return new Buffer(Mu, Beta.Bytes);
        }

        public void Reset()
        {
            Register(Mu.Rod.Signal);
        }

        private void Register(BigInteger Omicron)
        {
            Tau.WaitOne();
            if (Omicron == 0) throw new Exception("Omicron Zero");

            Mu.Charge(Omicron);
            if (Mu.Sigma < 4)
                throw new Exception("Sigma < 4");
            Mu.Induce();
            Mu.Torque();
            Mu.Amplify();
            Mu.Sustain();
            Mu.React();

            Enqueue(new byte[] { 0x00 });
            Enqueue(Omicron.ToByteArray());
            Enqueue(Mu.Sigma.ToByteArray());
            Enqueue(Mu.Cone.Channel.ToByteArray());
            Enqueue(Mu.Rod.Channel.ToByteArray());
            Enqueue(Mu.Rod.Kappa.ToByteArray());
            Tau.ReleaseMutex();
        }

        public BigInteger Op(String Beta)
        {
            byte[] Iota = Encoding.Unicode.GetBytes(Beta);
            BigInteger Omicron = new BigInteger(Iota);
            return Omicron;
        }

        public void Beta(BigInteger Omicron)
        {
            Tau.WaitOne();
            if (Mu.Omicron == 0) throw new Exception("Mu Omicron Zero");
            if (Omicron == 0) throw new Exception("Iota Zero");

            BigInteger Lambda = Mu.Rod.Torque(Omicron);
            Lambda = Mu.Rod.Amplify(Lambda);
            Lambda = Mu.Rod.Sustain(Lambda);
            Mu.Rod.React(Lambda, Mu.Cone.Dynamo);
            Mu.Cone.React(Lambda, Mu.Rod.Dynamo);

            Enqueue(new byte[] { 0x01 });
            Enqueue(Omicron.ToByteArray());
            Enqueue(Mu.Cone.Dynamo.ToByteArray());
            Enqueue(Mu.Rod.Dynamo.ToByteArray());
            Enqueue(Mu.Rod.Eta.ToByteArray());
            Enqueue(Mu.Cone.Eta.ToByteArray());
            Tau.ReleaseMutex();
        }

        public BigInteger Next()
        {
            byte[] Beta;
            TryDequeue(out Beta);
            return new BigInteger(Beta);
        }

        void Phi()
        {
            while (true)
            {
                Tau.WaitOne();
                if (Count > 0)
                {
                    byte[] OpCode;
                    TryDequeue(out OpCode);
                    if (OpCode[0] == 0x00)
                    {
                        BigInteger Beta = Next();
                        BigInteger Iota = Next();
                        if (Mu.Cone.Mu(Mu.Cone.Barn, Beta, Mu.Cone.Ring) != Iota) throw new Exception("Invalid Charge");

                        Beta = Next();
                        if (Beta != Mu.Cone.Channel) throw new Exception("Bad Cone Channel");

                        Beta = Next();
                        if (Beta != Mu.Rod.Channel) throw new Exception("Bad Rod Channel");

                        Beta = Next();
                        Iota = Mu.Rod.Induce(Iota);
                        Iota = Mu.Cone.Torque(Iota);
                        Iota = Mu.Cone.Amplify(Iota);
                        Iota = Mu.Cone.Sustain(Iota);
                        if (Mu.Rod.Mu(Iota, Mu.Cone.Channel, Mu.Rod.Channel) != Beta) throw new Exception("Invalid Reaction");
                        if(Count > 0) TryDequeue(out OpCode);
                    }

                    if (OpCode[0] == 0x01)
                    {
                        BigInteger Beta = Next();
                        BigInteger Iota = Next();
                        if (Iota != Mu.Cone.Dynamo) throw new Exception("Bad Cone Dynamo");
                        Iota = Next();
                        if (Iota != Mu.Rod.Dynamo) throw new Exception("Bad Rod Dynamo");

                        Beta = Mu.Rod.Torque(Beta);
                        Beta = Mu.Rod.Amplify(Beta);
                        Beta = Mu.Rod.Sustain(Beta);

                        Iota = Next();
                        if (Mu.Rod.Mu(Beta, Mu.Rod.Channel, Mu.Cone.Dynamo) != Iota) throw new Exception("Invalid Rod Eta");

                        Iota = Next();
                        if (Mu.Cone.Mu(Beta, Mu.Cone.Channel, Mu.Rod.Dynamo) != Iota) throw new Exception("Invalid Cone Eta");
                        if (Count > 0) TryDequeue(out OpCode);
                    }
                    if (Count != 0) throw new Exception("Execution Failure");
                }
                Tau.ReleaseMutex();
                Thread.Sleep(1000);
            }
        }
    }
}

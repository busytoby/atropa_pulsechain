﻿using Dysnomia.Domain.bin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Lib
{
    public class Buffer
    {
        public Faung Mu;
        public byte[]? Bytes;

        public Buffer(Daemon Beta)
        {
            if (Beta.Count < 5) throw new Exception("Daemon Queue Count < 5");
            BigInteger Rho, Upsilon, Ohm, Xi, Alpha;
            Rho = Beta.Next();
            Upsilon = Beta.Next();
            Ohm = Beta.Next();
            Xi = Beta.Next();
            Alpha = Beta.Next();
            Mu = new Faung(Rho, Upsilon, Ohm, Xi, Alpha);
        }

        public Buffer(BigInteger Rho, BigInteger Upsilon, BigInteger Ohm, BigInteger Xi, BigInteger Alpha)
        {
            Mu = new Faung(Rho, Upsilon, Ohm, Xi, Alpha);
        }

        public Buffer(Faung _Mu, byte[] Rho)
        {
            Mu = _Mu;
            Encode(Rho);
        }

        public void Clear()
        {
            Bytes = null;
        }

        public void Alpha(byte[] Omicron)
        {
            Alpha(new BigInteger(Omicron));
        }

        public void Alpha(BigInteger Omicron)
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            if (Omicron == 0) throw new Exception("Omicron Zero");

            Mu.Charge(Omicron);
            if (Mu.Sigma < 4)
                throw new Exception("Sigma < 4");
            Mu.Induce();
            Mu.Torque();
            Mu.Amplify();
            Mu.Sustain();
            Mu.React(false);
            if (Mu.Omega != Mu.Omicron) throw new Exception("NonParallel, Maybe Not A Greed Buffer ?");
            Logging.Log("Buffer", "Alpha Parallel: " + Mu.Omicron.ToString(), 2);
        }

        public void Beta(byte[] Omicron, bool Receiver)
        {
            Encode(Omicron);
            if (Bytes == null) throw new Exception("Null Bytes Alpha");
            Beta(new BigInteger(Receiver ? Bytes : Omicron));
        }

        public void Beta(BigInteger Omicron)
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            Logging.Log("Buffer", "Beta Light: " + Omicron.ToString(), 4);
            if (Mu.Omicron == 0) throw new Exception("Mu Omicron Zero");
            if (Omicron == 0) throw new Exception("Iota Zero");

            BigInteger Lambda = Mu.Rod.Torque(Omicron);
            Lambda = Mu.Rod.Amplify(Lambda);
            Lambda = Mu.Rod.Sustain(Lambda);
            Mu.Rod.React(Lambda, Mu.Cone.Dynamo);
            Mu.Cone.React(Lambda, Mu.Rod.Dynamo);
        }

        public void Pi()
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            BigInteger Lambda = Mu.Cone.Torque(Mu.Rod.Kappa);
            Lambda = Mu.Cone.Amplify(Lambda);
            Lambda = Mu.Cone.Sustain(Lambda);
            Mu.Rod.React(Lambda, Mu.Rod.Channel);
            Mu.Cone.React(Lambda, Mu.Cone.Channel);
        }

        public void Rho()
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            Mu.Upsilon = Mu.Cone.Torque(Mu.Rod.Eta);
            Mu.Ohm = Mu.Cone.Amplify(Mu.Upsilon);
            Mu.Pi = Mu.Cone.Sustain(Mu.Ohm);
            Mu.Cone.React(Mu.Pi, Mu.Cone.Dynamo);
            Mu.Omicron = Mu.Cone.Kappa;
            Mu.Rod.React(Mu.Pi, Mu.Rod.Dynamo);
            Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;
        }

        public void Upsilon(bool Phi = true)
        {
            Mu.Upsilon = Phi ? Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi : Mu.Upsilon ^ Mu.Ohm;
        }

        public void Upsilon(ref Faung Beta)
        {
            // stub additional record from beta
            Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi ^ Beta.Upsilon;
            Beta.Ohm = Beta.Ohm ^ Mu.Upsilon;
        }

        public void Upsilon(BigInteger Iota)
        {
            Mu.Upsilon = Mu.Upsilon ^ Iota;
        }

        public void Encode(BigInteger Rho)
        {
            Encode(Rho.ToByteArray());
        }

        public void Encode(byte[] Rho)
        {
            int _size = Rho.Length;
            if (Bytes == null) Bytes = new byte[_size];

            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");

            BigInteger Beta = Mu.Rod.Mu(Mu.Upsilon, Mu.Ohm, Mu.Pi);
            for (int i = 0; i < _size;)
            {
                byte[] Iota = Beta.ToByteArray();
                for (int j = 0; j < Iota.Length && i < _size; j++, i++)
                    Bytes[i] = (byte)(Iota[j] ^ Rho[i]);
                Beta = Mu.Rod.Mu(Beta, Mu.Ohm, Mu.Pi);
            }
        }

        public void Recode()
        {
            if (Bytes == null) throw new Exception("Cannot Recode Null Buffer");
            Encode(Bytes);
        }

        public override string ToString()
        {
            if (Bytes == null) return "";
            return new string(Bytes.Select(b => (char)b).ToArray());
        }
    }
}

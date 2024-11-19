using Dysnomia.Domain.bin;
using Dysnomia.Domain.World;
using Dysnomia.Lib;
using ExtensionMethods;
using System;
using System.Collections;
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
using Wallet;

namespace Dysnomia.Domain
{
    public class Oracle : Daemon
    {
        new public static string Name = "Oracle";
        new public static String Description = "Oracle Daemon";

        private Dictionary<byte[], Command> ProcessTable;

        public Oracle()
        {
            Logging.Log("Oracle", "New Oracle");
            Theta = new Living(Phi);
            ProcessTable = new Dictionary<byte[], Command>();
        }

        public Tare? Fi(Tare M)
        {
            short OpCode = M.OpCode();
            if (M.Count != 0) 
                throw new Exception("Mishandled Tare");
            return null;
        }

        public void ProcessString(String A)
        {
            Logging.Log("Oracle", "ProcessString: " + A, 3);
            Enqueue(new byte[] { 0x02 });
            Enqueue(Encoding.Default.GetBytes(A));
        }

        public dynamic ProcessStringAndWait(String A) {
            Command command = new Command(A);
            Tare? Pi;
            do {
                while(command.Theta.Out.Count > 0)
                    if(command.Theta.Out.TryDequeue(out Pi))
                        Logging.Log(Pi);
                Thread.Sleep(100);
            } while(command.Theta.Alive() || command.Theta.Out.Count > 0);
            return Command.Result;
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

        public BigInteger Op(String Beta)
        {
            byte[] Iota = Encoding.Unicode.GetBytes(Beta);
            BigInteger Omicron = new BigInteger(Iota);
            return Omicron;
        }

        protected override void Phi()
        {
            int _sleep = 20;
            byte[]? OpCode;

            BigInteger Iota, Omicron;
            byte[]? Lambda;
            String Xi;
            Tare? Pi;

            BigInteger ClientId;

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
                                if(!Aliases.Forward.ContainsKey(Wallet.Wallet._base)) {
                                    ProcessStringAndWait("deploy dysnomia/00b_vmreq.sol");
                                    ProcessStringAndWait("deploy wallet/00_base.sol þ þ VMRNG");
                                    ProcessStringAndWait("þ set random VMRNG");
                                    ProcessStringAndWait("deploy dysnomia/02c_shafactory.sol");
                                    ProcessStringAndWait("deploy dysnomia/03c_shiofactory.sol");
                                    ProcessStringAndWait("deploy dysnomia/04_yi.sol 02c_shafactory 03c_shiofactory random");

                                    ProcessStringAndWait("save");
                                    if(!Aliases.Forward.ContainsKey(Wallet.Wallet._base)) {
                                        throw new Exception("Unable To Initizalize þ Base");
                                    }
                                } else {
                                    byte[] _r = ProcessStringAndWait("þ get random");
                                    string RNGAddress = Encoding.Default.GetString(_r);
                                    ProcessStringAndWait("alias VMRNG " + RNGAddress);
                                }
                                break;
                            case 0x02:
                                Lambda = NextBytes();
                                if(Lambda == null) throw new Exception("Heap Corrupted");
                                Command command;
                                if(ProcessTable.ContainsKey(Lambda)) {
                                    command = ProcessTable[Lambda];
                                    if(command.Theta.Alive()) break;
                                } else {
                                    Xi = Encoding.Default.GetString(Lambda);
                                    Logging.Log("Oracle", "EXEC: " + Xi, 3);
                                    command = new Command(Xi);
                                }
                                if(command.Theta != null) {
                                    if(command.Theta.Alive()) ProcessTable.Add(Math.Random().ToByteArray(), command);
                                    else {
                                        if(ProcessTable.ContainsKey(Lambda)) ProcessTable.Remove(Lambda);
                                        while(command.Theta.In.Count > 0)
                                            if(command.Theta.In.TryDequeue(out Pi)) {
                                                byte[] From = Pi.NextBytes();
                                                byte[] Data = Pi.NextBytes();
                                                byte[] Priority = Pi.NextBytes();
                                                if(Pi.Count > 0) throw new Exception("Bad Command Input");
                                                Enqueue(Data);
                                            }
                                        while(command.Theta.Out.Count > 0)
                                            if(command.Theta.Out.TryDequeue(out Pi))
                                                Logging.Log(Pi);
                                    }
                                }
                                break;
                            default:
                                while (Count < 5) Thread.Sleep(100);
                                StdCall(OpCode);
                                break;
                        }

                        if (Count > 0) TryDequeue(out OpCode);
                    }
                    if (Count == 0) _sleep *= 2;
                }
                if (_sleep > 1551) _sleep = 1551;
                Thread.Sleep(_sleep);
                if (ProcessTable.Count > 0)
                    lock (Tau)
                        foreach (byte[] Key in ProcessTable.Keys)
                        {
                            while(ProcessTable[Key].Theta.Out.Count > 0)
                                if(ProcessTable[Key].Theta.Out.TryDequeue(out Pi))
                                    Logging.Log(Pi);
                            Enqueue(new byte[] { 0x02 });
                            Enqueue(Key);
                        }
            }
        }

        protected void StdCall(byte[] OpCode)
        {
            BigInteger Upsilon;
            BigInteger ClientId, ClientIdCheck;

            byte[] Code;
            byte[] Bytes;
            String DataString;

            ClientId = Next();
            Code = NextBytes();
            Bytes = NextBytes();
            ClientIdCheck = Next();

            if (ClientId != ClientIdCheck) throw new Exception("OpCode " + Convert.ToHexString(OpCode) + " ClientId Error");
            switch (OpCode[0])
            {
                default:
                    throw new Exception("Not Implemented");
            }
        }
    }
}

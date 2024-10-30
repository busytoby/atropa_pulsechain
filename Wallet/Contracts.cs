using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace Wallet
{
    public static class Contracts
    {
        static public string Solc_bin;
        static public string SolidityFolder;
        static public string OutputFolder;

        static Contracts() {            
        }

        public static void Init(string solc, string input, string output)
        {
            if (File.Exists(solc))
            {
                Solc_bin = solc;
            }
            else throw (new Exception("No Such File"));

            if (Directory.Exists(input))
            {
                SolidityFolder = input;
            }
            else throw (new Exception("No Such Folder"));

            if (Directory.Exists(output))
            {
                OutputFolder = output;
            }
            else throw (new Exception("No Such Folder"));
        }

        public static void Compile(string file)
        {
            Process _p = new Process();
            _p.StartInfo.FileName = Solc_bin;
            _p.StartInfo.Arguments = "--bin --abi --optimize --optimize-runs=200 --base-path " + SolidityFolder + " --overwrite -o " + OutputFolder + " --evm-version=shanghai " + file;
            //_p.StartInfo.RedirectStandardOutput = true;
            //_p.StartInfo.RedirectStandardInput = true;
            //_p.StartInfo.RedirectStandardError = true;
            _p.StartInfo.UseShellExecute = false;
            _p.StartInfo.CreateNoWindow = true;
            _p.Start();
            //string output = _p.StandardOutput.ReadToEnd();
            //output = _p.StandardError.ReadToEnd();
            _p.WaitForExit();
        }
    }
}

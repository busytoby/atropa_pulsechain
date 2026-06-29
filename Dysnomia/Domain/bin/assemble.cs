using System;
using System.Text;
using Dysnomia.Lib;

namespace Dysnomia.Domain.bin
{
    internal class assemble : Command
    {
        new public static String Name = "cmd_Assemble";
        new public static String Description = "Assemble [filename] [program_text] - Compile Folklore program onto disk";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length < 2)
            {
                Output(From, Encoding.Default.GetBytes("Usage: assemble <filename> <program_text>"), 6);
                return;
            }

            string filename = (string)Args[0];
            string programText = (string)Args[1];
            Output(From, Encoding.Default.GetBytes($"Assembling Folklore source into target inode '{filename}'..."), 6);

            bool success = Controller.LocalNetwork.CompileAndWriteFolklore(filename, programText);
            if (success)
            {
                Output(From, Encoding.Default.GetBytes($"SUCCESS: Program compiled and synced to '{filename}'"), 6);
            }
            else
            {
                Output(From, Encoding.Default.GetBytes($"FAILURE: Failed to compile program '{filename}'"), 6);
            }
        }
    }
}

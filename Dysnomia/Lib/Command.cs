using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Domain;
using Dysnomia.Domain.World;

namespace Dysnomia.Lib
{
    public class Command : Daemon
    {
        new public static string Name = "Command";
        new public static string Description = "Not Set Description";
        public dynamic[]? Args;
        public string? Alias;

        public Command() : base()
        {
            Args = null;
        }

        public Command(string Eta) {
            byte[] From = Encoding.Default.GetBytes(Name);

            Tokenize(Eta);
            object? EXE = null;
            Type[] Commands = GetCommands();

            Type C = Commands.FirstOrDefault(x => x.Name.ToLower().StartsWith(Name.ToLower()));
            if(C == null) {
                Alias = Name;
                C = Commands.FirstOrDefault(x => x.Name.ToLower().StartsWith("execute"));
            } else Alias = C.Name;
            EXE = Activator.CreateInstance(C, null);
            if(EXE == null) throw new Exception("Null Command Instance");
            if(Args == null) throw new Exception("Null Args");
            Command CEXE = (Command)EXE;
            CEXE.Alias = Alias;
            CEXE.Args = Args;
            CEXE.Theta = new Living();
            Theta = CEXE.Theta;
            if(EXE == null || CEXE.Theta == null) throw new Exception("Catastrophic Failure");
            CEXE.Theta.Run(CEXE.Phi);

            if (EXE == null || Theta == null)
                Logging.Log(Encoding.Default.GetString(From), Name + ": Not Found", 6);
        }

        private void Tokenize(string Eta)
        {
            List<string> _args = Eta.Split(" ").ToList();
            Name = _args[0];
            Args = _args.Skip(1).ToArray();
        }

        public Type[] GetCommands()
        {
            return Assembly.GetExecutingAssembly().GetTypes()
                .Where(t => string.Equals(t.Namespace, "Dysnomia.Domain.bin", StringComparison.Ordinal) && t.ReflectedType == null)
                .ToArray();
        }
    }
}

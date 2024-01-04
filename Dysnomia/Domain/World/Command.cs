using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Command
    {
        public string Name = "Command";
        public static String Description = "Not Set Description";
        public String[] Args;
        public Living Theta;

        public Command() { }

        public Command(String Eta) {
            byte[] From = Encoding.Default.GetBytes(Name);

            Tokenize(Eta);
            object EXE = null;
            Type[] Commands = GetCommands();

            foreach(Type C in Commands)
                if(C.Name.ToLower() == Name.ToLower())
                {
                    EXE = Activator.CreateInstance(C, null);
                    ((Command)EXE).Args = Args;
                    ((Command)EXE).Theta = new Living(((Command)EXE).Phi);
                    Theta = ((Command)EXE).Theta;
                    break;
                }

            if (EXE == null || Theta == null)
                Logging.Log(new Tare.MSG(From, Encoding.Default.GetBytes(Name + ": Not Found"), 6));
        }

        private void Tokenize(String Eta)
        {
            List<String> _args = Eta.Split(" ").ToList();
            Name = _args[0];
            Args = _args.Skip(1).ToArray();
        }

        public Type[] GetCommands()
        {
            return Assembly.GetExecutingAssembly().GetTypes()
                .Where(t => String.Equals(t.Namespace, "Dysnomia.Domain.bin", StringComparison.Ordinal))
                .ToArray();
        }

        protected virtual void Phi()
        {
            throw new Exception("Not Implemented");
        }
    }
}

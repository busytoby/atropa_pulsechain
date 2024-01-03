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
        public string Name;
        public static String Description = "Not Set Description";
        public String[] Args;
        public Living Theta;

        public Command() { }

        public Command(String Eta) {
            Tokenize(Eta);
            object EXE = null;
            Type[] Commands = GetCommands();

            foreach(Type C in Commands)
                if(C.Name.ToLower() == Name.ToLower())
                {
                    ConstructorInfo CI = C.GetConstructor(new Type[] { });
                    EXE = CI.Invoke(null);
                    ((Command)EXE).Name = Name;
                    ((Command)EXE).Args = Args;
                    ((Command)EXE).Theta = new Living(((Command)EXE).Phi);
                    Theta = ((Command)EXE).Theta;
                }

            if (EXE == null)
                throw new Exception("Command Not Found");
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

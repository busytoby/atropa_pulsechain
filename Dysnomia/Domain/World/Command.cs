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
        public Living Theta;
        public String[] Args = null;

        public Command() { }

        public Command(String Eta) {
            object EXE = null;
            Type[] Commands = GetCommands();

            foreach(Type C in Commands)
                if(C.Name.ToLower() == Eta.ToLower())
                {
                    ConstructorInfo CI = C.GetConstructor(new Type[] { });
                    EXE = CI.Invoke(null);
                    ((Command)EXE).Theta = new Living(((Command)EXE).Phi);
                    Theta = ((Command)EXE).Theta;
                }

            if (EXE == null)
                throw new Exception("Command Not Found");
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

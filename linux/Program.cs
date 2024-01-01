using Dysnomia.Domain;
using Dysnomia.Domain.World;
using System.Text;

namespace Pulse {
    public class ConsoleApp {
        static public OutputCaller Output;
	public delegate void OutputCaller(String A);

	static void Main(string[] args) {
	    Logging.Add(Input);
            Oracle O = new Oracle();
	    Output = O.ProcessString;
	    Fi.Listen(5555);
            Fi.Rho.Add(O.Fi);

	    string consoleinput;
	    while((consoleinput = Console.ReadLine()) != null)
		Output(consoleinput);
	}
	    
	public static void Input(Tare.MSG A) {
            Console.WriteLine("<" + Encoding.Default.GetString(A.From) + "> " + Encoding.Default.GetString(A.Data));
	}
    }
}


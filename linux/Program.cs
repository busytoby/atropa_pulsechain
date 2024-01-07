using Dysnomia;
using Dysnomia.Domain;
using Dysnomia.Domain.World;
using Dysnomia.Lib;
using System.Text;

namespace Pulse {
    public class ConsoleApp {
        static public OutputCaller? Output;
	public delegate void OutputCaller(String A);

	static void Main(string[] args) {
	    Logging.Add(Input);
	    Output = Controller.Oracle.ProcessString;
	    Controller.Fi.Listen(5555);

	    string? consoleinput;
	    while((consoleinput = Console.ReadLine()) != null)
		Output(consoleinput);
	}
	    
	public static void Input(Tare.MSG A) {
            Console.WriteLine("<" + Encoding.Default.GetString(A.From) + "> " + Encoding.Default.GetString(A.Data));
	}
    }
}


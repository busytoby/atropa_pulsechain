using Dysnomia;
using Dysnomia.Domain;
using Dysnomia.Domain.World;
using Dysnomia.Lib;
using System.Text;
using static ExtensionMethods.ExtensionMethods;

namespace Pulse {
    public class ConsoleApp {
        static public OutputCaller? Output;
	public delegate void OutputCaller(String A);

	static void Main(string[] args) {
	    Logging.Add(Input);
	    Output = Controller.Oracle.ProcessString;
	    Controller.Fi.Listen(5555);

            try {
                Console.In.Peek();
	        string? consoleinput;
	        while((consoleinput = Console.ReadLine()) != null)
		    Output(consoleinput);
            } catch (UnauthorizedAccessException) { }
            for(;;) Thread.Sleep(1000);

	}
	    
	public static Tare? Input(Tare A) {
	    Logging.MSG M = A.ToLogMSG();
            Console.WriteLine("<" + Encoding.Default.GetString(M.From) + "> " + Encoding.Default.GetString(M.Data));
	    return null;
	}
    }
}


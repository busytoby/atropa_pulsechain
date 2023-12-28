using Dysnomia.Domain;
using Dysnomia.Domain.World;
using System.Text;

namespace Pulse {
    public class ConsoleApp {
	static void Main(string[] args) {
	    Logging.Add(Input);
            Oracle O = new Oracle();
	}
	    
	public static void Input(Tare.MSG A) {
            Console.WriteLine("<" + Encoding.Default.GetString(A.From) + "> " + Encoding.Default.GetString(A.Data));
	}
    }
}


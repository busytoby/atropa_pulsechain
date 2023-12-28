using Dysnomia;
using Dysnomia.Domain;
using Dysnomia.Domain.World;
using System.Configuration;
using System.Data;
using System.Text;
using System.Windows;

namespace Pulse
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Bootstrap(object sender, StartupEventArgs e)
        {
            ShowApparitionWindow();

            Oracle O = new Oracle();
            Dysnomia.Domain.Buffer A = O.Encode("Test");
            Dysnomia.Domain.Buffer B = O.Encode("Test");
            String C = A.ToString();
            String D = B.ToString();
            O.Decode(A);
            O.Decode(B);
            O.Beta(Dysnomia.Math.Random());
            A = O.Encode("TSET");
            B = O.Encode("TSET");
            C = A.ToString();
            D = B.ToString();
            O.Decode(A);
            O.Decode(B);
            O.Reset();
            A = O.Encode("Test");
            B = O.Encode("Test");
        }

        private void ShowApparitionWindow()
        {
            if (Dysnomia.Apparition.Stub != true) throw new Exception("Zero Apparition");

            Application.Current.Dispatcher.Invoke((Action)delegate
            {
                Dysnomia.Apparition.Window = new Apparition.Retaliation.RetaliationWindow();
                Dysnomia.Apparition.Window.Show();
            });

            Logging.Add(Dysnomia.Apparition.Input);
            Logging.Log("Pulse", "Ready");
        }
    }

}
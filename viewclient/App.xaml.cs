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
            Logging.Add(Dysnomia.Apparition.Input);

            Oracle O = new Oracle();
            Dysnomia.Apparition.Output = O.ProcessString;

            Fi.Listen(5555);
            Fi.Rho.Add(O.Fi);

            Fi.Connect("127.0.0.1", 5555);
            //Fi.Connect("twee.se", 5555);
        }

        private void ShowApparitionWindow()
        {
            if (Dysnomia.Apparition.Stub != true) throw new Exception("Zero Apparition");

            Application.Current.Dispatcher.Invoke((Action)delegate
            {
                Dysnomia.Apparition.Window = new Apparition.Retaliation.RetaliationWindow();
                Dysnomia.Apparition.Window.Show();
            });

            Logging.Log("Pulse", "Ready");
        }
    }

}
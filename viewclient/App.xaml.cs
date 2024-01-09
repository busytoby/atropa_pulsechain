using Dysnomia;
using Dysnomia.Domain;
using Dysnomia.Lib;
using ExtensionMethods;
using System.Configuration;
using System.Data;
using System.Text;
using System.Windows;
using static ExtensionMethods.ExtensionMethods;

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
            Dysnomia.Apparition.Interpreter = Controller.Oracle.ProcessString;
            Controller.Fi.Listen(5555);
            DysnomiaTime T = DateTime.Now.ToDysnomia();
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
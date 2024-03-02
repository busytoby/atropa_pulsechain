#define DEMO
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
            Controller.Fi.Listen(5555);
        }

        private void ShowApparitionWindow()
        {
            if (Dysnomia.Apparition.Stub != true) throw new Exception("Zero Apparition");

            Logging.Add(Dysnomia.Apparition.Input);
            Application.Current.Dispatcher.Invoke((Action)delegate
            {
                Dysnomia.Apparition.Window = new Apparition.Retaliation.RetaliationWindow();
                Dysnomia.Apparition.Window.Show();
            });
            Dysnomia.Apparition.Interpreter = Controller.Oracle.ProcessString;

            Logging.Log("Pulse", "Ready");
        }
    }

}
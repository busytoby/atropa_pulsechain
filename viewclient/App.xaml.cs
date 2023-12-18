using Dysnomia;
using Dysnomia.Domain;
using System.Configuration;
using System.Data;
using System.Windows;

namespace Pulse
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private 錨 A;

        private void Bootstrap(object sender, StartupEventArgs e)
        {
            Task t0 = new Task(() => { A = new 錨(); });
            t0.Start();

            //Pulse.API Comptroller = new Pulse.API();
            //InitializeComponent();
            //StartThreads();

            Task t4 = new Task(() => { Beta(); });
            t4.Start();
        }

        private void Beta()
        {
            while (A == null || Zuo.Mu == null | A.Rho == null || A.Psi == null || A.Nu == null)
                Thread.Sleep(400);

            List<ည> B = Zuo.Mu.Nu;
            ໂຂ໌ C = A.Beta();
            if (Dysnomia.Apparition.Stub != true) throw new Exception("Zero Apparition");

            Application.Current.Dispatcher.Invoke((Action)delegate
            {
                Apparition.Retaliation.RetaliationWindow w = new Apparition.Retaliation.RetaliationWindow();
                w.Show();
            });
        }
    }

}
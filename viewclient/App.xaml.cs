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
        //private 錨 A;

        private void Bootstrap(object sender, StartupEventArgs e)
        {
            Oracle O = new Oracle();

            //Task t0 = new Task(() => { A = new 錨(); });
            //t0.Start();

            //Pulse.API Comptroller = new Pulse.API();
            //InitializeComponent();
            //StartThreads();

            //Task t4 = new Task(() => { Beta(); });
            //t4.Start();
        }

        /*
        private void Beta()
        {
            while (A == null || Zuo.Mu == null | A.Rho == null || A.Psi == null || A.Nu == null)
                Thread.Sleep(400);

            List<ည> B = Zuo.Mu.Nu;
            ໂຂ໌ C = A.Beta();
            if (Dysnomia.Apparition.Stub != true) throw new Exception("Zero Apparition");

            Application.Current.Dispatcher.Invoke((Action)delegate
            {
                Dysnomia.Apparition.Window = new Apparition.Retaliation.RetaliationWindow();
                Dysnomia.Apparition.Window.Show();
            });

            Dysnomia.Apparition.Tau.WaitOne();
            Dysnomia.Apparition.MsgQueue.Enqueue(new Tuple<String, String>("Test", "Ready"));
            Dysnomia.Apparition.Tau.ReleaseMutex();
            Dysnomia.Apparition.Tau.WaitOne();
            Dysnomia.Apparition.MsgQueue.Enqueue(new Tuple<String, String>("Test", "Two"));
            Dysnomia.Apparition.MsgQueue.Enqueue(new Tuple<String, String>("Ready", "Three"));
            Dysnomia.Apparition.Tau.ReleaseMutex();
        }
        */
    }

}
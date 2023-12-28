using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace Apparition.Retaliation
{
    public partial class RetaliationWindow : Window 
    {
        public RetaliationWindow()
        {
            InitializeComponent();
            Task t0 = new Task(() => { Frame(); });
            t0.Start();
        }

        private void Frame()
        {
            Tuple<String, String> Beta;
            while (true)
            {
                while (Dysnomia.Apparition.MsgQueue.Count > 0)
                {
                    lock (Dysnomia.Apparition.Tau) Beta = Dysnomia.Apparition.MsgQueue.Dequeue();
                    Application.Current.Dispatcher.Invoke((Action)delegate { 
                        TerminalOutput.AppendText("<" + Beta.Item1 + "> " + Beta.Item2 + "\n");
                        TerminalOutput.ScrollToEnd();
                    });
                }
                Thread.Sleep(111);
            }
        }

        private void RetaliationWindow_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftAlt) && e.ChangedButton == MouseButton.Left && e.ButtonState == MouseButtonState.Pressed)
                this.DragMove();
        }
    }
}

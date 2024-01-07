using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Threading;
using WinRT;

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
            Tare.MSG Beta;
            while (true)
            {
                while (Dysnomia.Apparition.MsgQueue.Count > 0)
                {
                    lock (Dysnomia.Apparition.Tau)
                        if (Dysnomia.Apparition.MsgQueue.TryDequeue(out Beta))
                            Application.Current.Dispatcher.Invoke((Action)delegate
                            {
                                TerminalOutput.AppendText("<" + Encoding.Default.GetString(Beta.From) + "> " + Encoding.Default.GetString(Beta.Data) + "\n");
                                TerminalOutput.ScrollToEnd();
                            });
                }
                Thread.Sleep(111);
            }
        }

        private void Retaliation_KeyDown(object sender, KeyEventArgs e)
        {
            if (Dysnomia.Apparition.Interpreter == null) throw new Exception("No Interpretation Handler");

            if(e.Key == Key.Enter)
            {
                Dysnomia.Apparition.Interpreter(new TextRange(sender.As<RichTextBox>().Document.ContentStart, sender.As<RichTextBox>().Document.ContentEnd).Text.Trim());
                sender.As<RichTextBox>().Document.Blocks.Clear();
                e.Handled = true;
            }
        }

        private void RetaliationWindow_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftAlt) && e.ChangedButton == MouseButton.Left && e.ButtonState == MouseButtonState.Pressed)
                this.DragMove();
        }
    }
}

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
            while (true)
            {
                while (Dysnomia.Apparition.MsgQueue.Count > 0)
                {
                    Dysnomia.Apparition.Tau.WaitOne();
                    Tuple<String, String> Beta = Dysnomia.Apparition.MsgQueue.Dequeue();
                    Application.Current.Dispatcher.Invoke((Action)delegate
                    {
                        TerminalOutput.AppendText("<" + Beta.Item1 + "> " + Beta.Item2 + "\n");
                    });
                    Dysnomia.Apparition.Tau.ReleaseMutex();
                }
                Thread.Sleep(111);
            }
        }

        private void RetaliationWindow_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftAlt) && e.ChangedButton == MouseButton.Left && e.ButtonState == MouseButtonState.Pressed)
                this.DragMove();
            /*
            if (Keyboard.IsKeyDown(Key.LeftAlt) && e.ChangedButton == MouseButton.Middle && e.ButtonState == MouseButtonState.Pressed)
            {
                isResizing = true;
                Point resizingOrigin = e.GetPosition(this);
                resizingOriginX = (int)resizingOrigin.X;
                resizingOriginY = (int)resizingOrigin.Y;
                if (resizingOriginX < Width / 2 && resizingOriginY < Width / 2) resizingCorner = 0;
                if (resizingOriginX > Width / 2 && resizingOriginY < Width / 2) resizingCorner = 1;
                if (resizingOriginX > Width / 2 && resizingOriginY > Width / 2) resizingCorner = 2;
                if (resizingOriginX < Width / 2 && resizingOriginY > Width / 2) resizingCorner = 3;
                this.MouseMove += new MouseEventHandler(PerceiveHUDWindow_MouseResize);
            }
            */
        }

    }
}

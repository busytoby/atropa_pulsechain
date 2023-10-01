using System.Net.Http;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Dynamic;
using System.IO;
using System.Security.Policy;
using Microsoft.Data.Sqlite;
using System.Runtime.InteropServices;
using Microsoft.VisualBasic;
using System.Windows.Media.TextFormatting;

namespace Pulse
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private bool UIUpdating = false;

        public MainWindow()
        {

            Pulse.API Comptroller = new Pulse.API();

            InitializeComponent();
            StartThreads();
        }

        private void StartThreads()
        {  
            Action su = new Action(() => { StageUI(); });
            Action<object> sc = (object o) => { StageUI2(); };
            Task t1 = new Task(su);
            t1.ContinueWith(sc);
            t1.Start();
        }

        private void StageUI2()
        {
            while (API.UIStage != 1) System.Threading.Thread.Sleep(1000);

            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.BeginInvoke(() => { StageUI2(); });
                return;
            }

            sp.Children.Clear();
            Canvas cv = new Canvas();
            cv.Background = Brushes.Thistle;

            BezierSegment curve = new BezierSegment(new Point(4, 4), new Point(12, 12), new Point(24, 12), false);
            PathGeometry pg = new PathGeometry();
            PathFigure pf = new PathFigure();
            pf.StartPoint = new Point(4, 4);
            pg.Figures.Add(pf);
            pf.Segments.Add(curve);
            System.Windows.Shapes.Path p = new System.Windows.Shapes.Path();
            p.Fill = Brushes.Green;
            p.Stroke = Brushes.Crimson;
            p.StrokeThickness = 8;
            p.Data = pg;
            cv.Children.Add(p);
            sp.Children.Add(cv);

            API.UIStage = 2;
        }


        private void StageUI()
        {
            while (API.Tokens.Count == 0) System.Threading.Thread.Sleep(1000);

            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.BeginInvoke(() => { StageUI(); });
                return;
            }
            
            PopulateSP();
        }

        public int PopulateSP(int offset = 0)
        {
            if (UIUpdating) return offset;
            UIUpdating = true;

            int i = offset;
            for(; i < API.Tokens.Count; i++) {
                API.Token tk = API.Tokens[i];
                Border B = new Border();
                B.Background = Brushes.White;
                B.BorderBrush = Brushes.Thistle;
                B.BorderThickness = new Thickness(1);
                TextBlock T = new TextBlock();
                T.Foreground = Brushes.Black;
                T.FontSize = 16;
                String DisplayName = tk.name;
                if(API.Aliases.ContainsKey(tk.contractAddress))
                    DisplayName = API.Aliases[tk.contractAddress];
                T.Text = String.Format("{0}\t{1} ({2})\t{3}", tk.contractAddress, DisplayName, tk.symbol, tk.balance);
                B.Child = T;
                MouseBinding mb = new MouseBinding();
                mb.MouseAction = MouseAction.LeftClick;
                //mb.Command = row_mbClicked;
                // T.InputBindings.Add();
                sp.Children.Add(B);
            }
            UIUpdating = false;
            return i;
        }
    }
}
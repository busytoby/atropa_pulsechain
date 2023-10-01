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
using System.Windows.Media.Animation;

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
            Action<object> sc = (object o) => { System.Threading.Thread.Sleep(2000); StageUI2(); };
            Task t1 = new Task(su);
            t1.ContinueWith(sc);
            t1.Start();
        }

        private void StageUI2()
        {
            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.BeginInvoke(() => { StageUI2(); });
                return;
            }

            sp.Children.Clear();
            Canvas cv = new Canvas();
            cv.Background = Brushes.Thistle;

            TextBlock tb = new TextBlock();
            tb.Text = "Atropa";
            Canvas.SetTop(tb, 40);
            Canvas.SetLeft(tb, 30);            
            cv.Children.Add(tb);
            sp.Children.Add(cv);

            Action su = new Action(() => { System.Threading.Thread.Sleep(2000); PopulateUI2(cv); });
            Task t1 = new Task(su);
            t1.Start();
        }

        private void AddToCanvas(Canvas cv, UIElement e)
        {
            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.BeginInvoke(() => { AddToCanvas(cv, e); });
                return;
            }

            cv.Children.Add(e);
        }

        private void PopulateUI2(Canvas cv, List<string> DisplayedTokens = null)
        {
            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.BeginInvoke(() => { PopulateUI2(cv, DisplayedTokens); });
                return;
            }

            int PLPTokenCount = 0;
            foreach (API.Token t in API.Tokens)
                if (t.symbol == "PLP") PLPTokenCount++;

            if(DisplayedTokens == null)
                DisplayedTokens = new List<string>();
            int ntdTop = 40;
            int ntdLeft = 80;
            while (DisplayedTokens.Count < PLPTokenCount || API.UIStage != 1)
            {
                System.Threading.Thread.Sleep(1000);
                foreach (API.Token t in API.Tokens)
                {
                    if (!DisplayedTokens.Contains(t.contractAddress) && API.Aliases.ContainsKey(t.contractAddress))
                    {
                        TextBlock tbp = new TextBlock();
                        tbp.Text = API.Aliases[t.contractAddress];
                        ntdTop = ((1 + DisplayedTokens.Count) * 15) + 40;
                        Canvas.SetTop(tbp, ntdTop);
                        Canvas.SetLeft(tbp, ntdLeft);
                        AddToCanvas(cv, tbp);
                        DisplayedTokens.Add(t.contractAddress);
                        Action su = new Action(() => { System.Threading.Thread.Sleep(400); PopulateUI2(cv, DisplayedTokens); });
                        Task t1 = new Task(su);
                        t1.Start();
                        return;
                    }
                }
            }

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
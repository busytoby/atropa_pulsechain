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
using System.Linq.Expressions;
using System.Numerics;
using Dysnomia;
using Dysnomia.Domain;

namespace Pulse
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private bool UIUpdating = false;
        private 錨 A;

        public MainWindow()
        {
            Task t0 = new Task(() => { A = new 錨(); });
            t0.Start();

            //Pulse.API Comptroller = new Pulse.API();
            InitializeComponent();
            //StartThreads();

            Task t4 = new Task(() => { Beta(); });
            t4.Start();           
        }

        private void Beta()
        {
            while (A == null || Zuo.Mu == null | A.Rho == null || A.Psi == null || A.Nu == null)
                Thread.Sleep(400);

            OpCode Alpha = new OpCode(A.Rho.Rho.Psi.Theta.Xi[0]);
            OpCode Beta = new OpCode(A.Rho.Rho.Psi.Theta.Xi[0], "Atropa");
            OpCode Gamma = new OpCode(A.Rho.Rho.Psi.Theta.Xi[1], "X");
            OpCode Delta = new OpCode(A.Rho.Rho.Psi.Theta.Xi[1], Alpha.BinCode.ToString());

            int i = 99;
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
            tb.MouseDown += new MouseButtonEventHandler(tbClick);
            cv.Children.Add(tb);
            sp.Children.Add(cv);

            Action su = new Action(() => { System.Threading.Thread.Sleep(400); PopulateUI2(cv); });
            Task t1 = new Task(su);
            t1.Start();

            Action sl = new Action(() => { PopulateLiquidities(); });
            Task t2 = new Task(sl);
            t2.Start();
        }

        private void tbClick(object sender, MouseButtonEventArgs e)
        {
            int i = 99;
        }

        private void PopulateLiquidities()
        {
            int count = 1;
            dynamic t;
            do
            {
                t = API.GetTokenHolders(API.AtropaContract, count);
                foreach(dynamic ti in t["result"])
                {
                    try {
                        dynamic t2 = API.GetToken(ti["address"].ToString());
                        if (t2["result"] != null && t2["result"]["symbol"].ToString() == "PLP")
                        {
                            string Alias = SQLite.Query.GetAlias(ti["address"].ToString());
                            if (Alias.Length == 0)
                            {
                                dynamic t4 = API.GetFirstTransaction(ti["address"].ToString());
                                string c1 = t4["result"][1]["to"].ToString();
                                string c2 = t4["result"][2]["to"].ToString();

                                dynamic t3 = API.GetAccountHoldings(ti["address"].ToString());
                                int ca = 0;
                                int cb = 0;
                                for (int i = 0; i < t3["result"].Count; i++)
                                {
                                    if (t3["result"][i]["contractAddress"].ToString() == c1) ca = i;
                                    if (t3["result"][i]["contractAddress"].ToString() == c2) cb = i;
                                }
                                Alias = String.Format("{0} ({1}) - {2} ({3}) PLP", t3["result"][ca]["name"].ToString(), t3["result"][ca]["symbol"].ToString(), t3["result"][cb]["name"].ToString(), t3["result"][cb]["symbol"].ToString());
                                SQLite.Query.InsertAlias(ti["address"].ToString(), Alias);
                                API.Aliases.Add(ti["address"].ToString(), Alias);
                                SQLite.Query.InsertContractHoldings(ti["address"].ToString(), t3["result"][ca]["contractAddress"].ToString(), t3["result"][ca]["balance"].ToString());
                                SQLite.Query.InsertContractHoldings(ti["address"].ToString(), t3["result"][cb]["contractAddress"].ToString(), t3["result"][cb]["balance"].ToString());
                            }
                        }
                    } catch (Exception ex)
                    {
                        int r = 33;
                    }
                }
                
            } while (t.Count == 100);
            int v = 99;
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
                        Action su = new Action(() => { System.Threading.Thread.Sleep(40); PopulateUI2(cv, DisplayedTokens); });
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
            while (API.Tokens.Count == 0) System.Threading.Thread.Sleep(400);

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
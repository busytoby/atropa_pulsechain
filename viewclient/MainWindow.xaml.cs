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

namespace Pulse
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public string AtropaContract = "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6";

        private List<API.Token> Tokens;
        private SQLite.Query Querier;

        public MainWindow()
        {
            Tokens = new List<API.Token>();
            Querier = new SQLite.Query();

            InitializeComponent();
            GetTokens(AtropaContract);
            PopulateSP();
            GetTokenDatas();
        }

        //public void rowmb

        public void PopulateSP()
        {
            foreach(API.Token tk in Tokens)
            {
                Border B = new Border();
                B.Background = Brushes.White;
                B.BorderBrush = Brushes.Thistle;
                B.BorderThickness = new Thickness(1);
                TextBlock T = new TextBlock();
                T.Foreground = Brushes.Black;
                T.FontSize = 16;
                T.Text = String.Format("{0}\t{1} ({2})\t{3}", tk.contractAddress, tk.name, tk.symbol, tk.balance);
                B.Child = T;
                MouseBinding mb = new MouseBinding();
                mb.MouseAction = MouseAction.LeftClick;
                //mb.Command = row_mbClicked;
                // T.InputBindings.Add();
                sp.Children.Add(B);
            }
        }

        public void GetTokenDatas()
        {
            foreach (API.Token tk in Tokens)
            {
                int a = 44;
                if (tk.symbol != "PLP") continue;
                List<Dictionary<string, string>> t = API.GetAccountHoldings(tk.contractAddress);
                int v = 99;

            }
        }

        public void GetTokens(String ContractAddress)
        {
            try
            {
                List<Dictionary<string, string>> t = API.GetAccountHoldings(ContractAddress);

                foreach (Dictionary<string, string> tkd in t)
                {
                    API.Token tk = new API.Token();
                    tk.holder = ContractAddress;
                    tk.balance = tkd["balance"];
                    tk.contractAddress = tkd["contractAddress"];
                    tk.decimals = tkd["decimals"];
                    tk.name = tkd["name"];
                    tk.symbol = tkd["symbol"];
                    tk.type = tkd["type"];

                    SqliteCommand chk = SQLite.Query.SelectTokensByAddress(tk.contractAddress);
                    using (var reader = chk.ExecuteReader())
                    {
                        if (!reader.HasRows)
                        {
                            SQLite.Query.InsertToken(tk); // unchecked
                            Tokens.Add(tk);
                        }
                        else
                        {
                            Tokens.Add(tk);
                            while (reader.Read())
                            {
                                if (reader.GetString(3) == tk.balance || tk.balance == null)
                                    continue;
                                else
                                {
                                    int k = 44;
                                }
                            }
                        }
                    }
                    chk.Dispose();
                    chk = SQLite.Query.SelectAsset(ContractAddress, tk.contractAddress);
                    using (var reader = chk.ExecuteReader())
                    {
                        if (!reader.HasRows)
                        {
                            SQLite.Query.InsertContractHoldings(ContractAddress, tk.contractAddress, tk.balance);
                        }
                        else
                        {
                            while (reader.Read())
                            {
                                if (reader.GetString(3) == tk.balance || tk.balance == null)
                                    continue;
                                else
                                {
                                    int k = 44;
                                }
                            }
                        }
                    }

                    tk = new API.Token();
                    tk.holder = ContractAddress;
                }
            }
            catch (Exception ex)
            {
                int e = 44;
            }
        }
    }
}
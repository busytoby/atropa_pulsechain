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

namespace Pulse
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public class Token
        {
            public string balance;
            public string contractAddress;
            public string decimals;
            public string name;
            public string symbol;
            public string type;
        }

        public List<Token> Tokens;
        bool TokenComplete = false;

        public MainWindow()
        {
            Tokens = new List<Token>();
            InitializeComponent();
            InitDB();
            GetTokens();
            PopulateSP();
            //GetTokenDatas();
        }

        public void PopulateSP()
        {
            foreach(Token tk in Tokens)
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
                sp.Children.Add(B);
            }
        }

        public void GetTokenDatas()
        {
            while (!TokenComplete) 
                System.Threading.Thread.Sleep(1000);

            foreach (Token tk in Tokens)
            {
                HttpClient client = new HttpClient();
                Task<string> ts = client.GetStringAsync(new Uri(
                    String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", tk.contractAddress)));
                ts.Wait();
                client.Dispose();
                string s = ts.Result;
                string[] t = s.Split(',');
                int v = 99;

                client = new HttpClient();
                ts = client.GetStringAsync(new Uri(
                    String.Format("https://scan.pulsechain.com/api?module=token&action=getTokenHolders&contractaddress={0}&page=1&offset=100", tk.contractAddress)));
                ts.Wait();
                client.Dispose();
                s = ts.Result;
                t = s.Split(',');
                v = 99;
            }
        }

        public void InitDB()
        {
            string dbpath = "sqlite.db";
            using (var db = new SqliteConnection($"Filename={dbpath}"))
            {
                db.Open();

                string tableCommand = "CREATE TABLE IF NOT " +
                    "EXISTS Tokens (Address NVARCHAR(256) PRIMARY KEY, " +
                    "Symbol NVARCHAR(256) NULL," +
                    "Name NVARCHAR(256) NULL," +
                    "Balance NVARCHAR(256) NULL," +
                    "Decimals NVARCHAR(8) NULL," +
                    "Type NVARCHAR(256) NULL)";

                var createTable = new SqliteCommand(tableCommand, db);

                createTable.ExecuteReader();
            }
        }

        public void GetTokens()
        {
            string dbpath = "sqlite.db";
            using (var db = new SqliteConnection($"Filename={dbpath}"))
            {
                db.Open();

                try
                {
                    HttpClient client = new HttpClient();
                    Task<string> ts = client.GetStringAsync(new Uri("https://scan.pulsechain.com/api?module=account&action=tokenlist&address=0x7a20189B297343CF26d8548764b04891f37F3414"));
                    ts.Wait();
                    //string s = await client.GetStringAsync(new Uri("https://scan.pulsechain.com/api?module=account&action=tokenlist&address=0x7a20189B297343CF26d8548764b04891f37F3414"));
                    string s = ts.Result;
                    client.Dispose();
                    string[] t = s.Split(',');
                    Token tk = new Token();
                    tk.balance = t[1].Split("\"")[5];
                    int c = 1;
                    for (int i = 2; i < t.Count(); i++)
                    {
                        string[] tkt = t[i].Split("\"");
                        if (tkt[1] == "balance")
                            tk.balance = tkt[3];
                        else if (tkt[1] == "contractAddress")
                            tk.contractAddress = tkt[3];
                        else if (tkt[1] == "decimals")
                            tk.decimals = tkt[3];
                        else if (tkt[1] == "name")
                            tk.name = tkt[3];
                        else if (tkt[1] == "symbol")
                            tk.symbol = tkt[3];
                        else if (tkt[1] == "type")
                        {
                            tk.type = tkt[3];

                            SqliteCommand chk = new SqliteCommand(String.Format("Select * From Tokens Where Address = \"{0}\";", tk.contractAddress), db);
                            using (var reader = chk.ExecuteReader())
                            {
                                if (!reader.HasRows)
                                {
                                    SqliteCommand ins = new SqliteCommand(String.Format("INSERT INTO Tokens (Address, Symbol, Name, Balance, Decimals, Type) VALUES (\"{0}\", \"{1}\", \"{2}\", \"{3}\", \"{4}\", \"{5}\");",
                                        tk.contractAddress, tk.symbol, tk.name, tk.balance, tk.decimals, tk.type), db);

                                    ins.ExecuteNonQuery();
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

                            tk = new Token();
                        }
                        c++;
                        if (c > 5) c = 0;
                    }
                }
                catch (Exception ex)
                {
                    int e = 44;
                }
            }
            TokenComplete = true;
        }
    }
}
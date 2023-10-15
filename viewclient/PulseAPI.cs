using Microsoft.Data.Sqlite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Text.Json.Nodes;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using System.Timers;

namespace Pulse
{
    public class API
    {
        public static string AtropaContract = "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6";

        private static SQLite.Query Querier;
        public static bool UIUpdating = false;

        public static int UIStage = 0;
        private static System.Timers.Timer rateLimitingTimer = null;
        public static List<API.Token> Tokens = null;       
        public static Dictionary<String, String> Aliases;

        private static int rateLimit = 100;

        public API()
        {
            Tokens = new List<API.Token>();
            Querier = new SQLite.Query();
            Aliases = SQLite.Query.GetAliases();

            StartThreads();
        }

        private void StartThreads()
        {
            Action ac = new Action(() => { API.GetTokens(AtropaContract); });
            //Action<object> sp = (object o) => { PopulateSP(); };
            Action<object> td = (object o) => { API.GetTokenDatas(); UIStage = 1; };
            Task t1 = new Task(ac);
            Task t2 = t1.ContinueWith(td);
            //t2.ContinueWith(td);
            t1.Start();

            /* might or might not retain some hook
            Action su = new Action(() => { StageUI(); });
            Task t3 = Task.Run(su);
            */
        }

        public class Token
        {
            public string balance;
            public string contractAddress;
            public string decimals;
            public string name;
            public string symbol;
            public string type;
            public string holder;
        }

        private static void RateLimit()
        {
            if(rateLimitingTimer != null)
                while (rateLimitingTimer.Enabled == true) System.Threading.Thread.Sleep(40);
            else { 
                rateLimitingTimer = new System.Timers.Timer(rateLimit);
                rateLimitingTimer.Elapsed += RateEndEvent;
                rateLimitingTimer.AutoReset = false;
                rateLimitingTimer.Enabled = false;
            }
            if (rateLimitingTimer.Enabled == false) rateLimitingTimer.Enabled = true;
            System.Threading.Thread.Sleep(rateLimit);
        }

        private static void RateEndEvent(Object src, ElapsedEventArgs e)
        {
            rateLimitingTimer.Stop();
            rateLimitingTimer.Enabled = false;
            return;
        }

        private static dynamic GetURI(Uri uri)
        {
            while (true)
            {
                RateLimit();
                HttpClient client = new HttpClient();
                client.DefaultRequestHeaders.UserAgent.ParseAdd("ATROPA/1.0 (compatible; PulsescanAPI/220930)");
                client.DefaultRequestHeaders.Add("atropaclientid", "this will be unique later");
                try
                {
                    Task<string> ts = client.GetStringAsync(uri);
                    ts.Wait();
                    client.Dispose();
                    string s = ts.Result;
                    dynamic d = JsonObject.Parse(s);
//                    JsonConverter
                    rateLimit = 100;
                    return d;
                    //return SanitizeResult(s.Split(','));
                }
                catch (Exception ex)
                {
                    rateLimit *= 2;
                    if (rateLimit > 8000) rateLimit = 8000;
                    client.Dispose();
                    int e = 44;
                }
            }
        }

        public static dynamic GetToken(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=token&action=getToken&contractaddress={0}&page=1&offset=100", ContractAddress)));
        }

        public static dynamic GetTokenList(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", ContractAddress)));
        }

        public static dynamic GetTokenHolders(string ContractAddress, int offset = 1)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=token&action=getTokenHolders&contractaddress={0}&page={1}&offset=100", ContractAddress, offset.ToString()))); ;
        }

        public static dynamic GetAccountHoldings(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", ContractAddress)));
        }

        public static dynamic GetFirstTransaction(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=account&action=txlistinternal&address={0}&sort=asc", ContractAddress)));
        }

        /*
        public static List<Dictionary<string, string>> SanitizeResult(string[] s)
        {
            if (s[0] != "{\"message\":\"OK\"") throw new Exception("Doesn't Look OK");
            string[] s1chk = s[1].Split('\"');
            if (s1chk[1] != "result") throw new Exception("Doesn't Look Result !");

            List<Dictionary<string, string>> R = new List<Dictionary<string, string>>();
            Dictionary<string, string> T = new Dictionary<string, string>();
            if(s1chk.Count() == 7)
                T[s1chk[3]] = s1chk[5];

            int s3chk = 0;
            for (int i = 2; i < s.Count(); i++)
            {
                string[] s2chk = s[i].Split('\"');
                if (s2chk.Count() != 5) throw new Exception("Doesn't Look Check !");
                if (s2chk[0] == "{" || s2chk[4] == "}]" || (s1chk.Count() != 7 && s2chk[4] == "}")) {
                    R.Add(T);
                    if (s3chk == 0) s3chk = T.Count();
                    if (s3chk != T.Count() && s.Count() < (i + 2)) 
                        throw new Exception("Doesn't Look Count !");
                    if (s1chk.Count() != 7 && s2chk[4] == "}")
                    {
                        T[s2chk[1]] = s2chk[3];
                        break;
                    }
                    T = new Dictionary<string, string>();
                }
                T[s2chk[1]] = s2chk[3];
            }

            return R;
        }
        */

        public static void GetTokenDatas()
        {
            foreach (API.Token tk in API.Tokens)
            {
                if (tk.symbol != "PLP") continue;
                dynamic t = API.GetAccountHoldings(tk.contractAddress);

                string Alias = SQLite.Query.GetAlias(tk.contractAddress);
                if (Alias.Length == 0)
                {
                    dynamic t4 = API.GetFirstTransaction(tk.contractAddress);
                    string c1 = t4["result"][1]["to"].ToString();
                    string c2 = t4["result"][2]["to"].ToString();
                    if (c1 == c2) c2 = t4["result"][3]["to"].ToString();
                    if (c1 == c2) c2 = t4["result"][4]["to"].ToString();
                    if (c1 == c2) c2 = t4["result"][5]["to"].ToString();
                    if (c1 == c2) c2 = t4["result"][6]["to"].ToString();
                    if (c1 == c2) 
                        throw new Exception("LP Pair Detection");

                    int ca = 0;
                    int cb = 0;
                    for (int i = 0; i < t["result"].Count; i++)
                    {
                        if (t["result"][i]["contractAddress"].ToString() == c1) ca = i;
                        if (t["result"][i]["contractAddress"].ToString() == c2) cb = i;
                    }
                    Alias = String.Format("{0} ({1}) - {2} ({3}) PLP", t["result"][ca]["name"].ToString(), t["result"][ca]["symbol"].ToString(), t["result"][cb]["name"].ToString(), t["result"][cb]["symbol"].ToString());
                    SQLite.Query.InsertAlias(tk, Alias);
                    API.Aliases.Add(tk.contractAddress, Alias);
                    SQLite.Query.InsertContractHoldings(tk.contractAddress, t["result"][ca]["contractAddress"].ToString(), t["result"][ca]["balance"].ToString());
                    SQLite.Query.InsertContractHoldings(tk.contractAddress, t["result"][cb]["contractAddress"].ToString(), t["result"][cb]["balance"].ToString());
                }
            }
        }

        public static void GetTokens(String ContractAddress)
        {
            try
            {
                dynamic t = API.GetAccountHoldings(ContractAddress);

                foreach (dynamic tkd in t["result"])
                {
                    API.Token tk = new API.Token();
                    tk.holder = ContractAddress;
                    tk.balance = tkd["balance"].ToString();
                    tk.contractAddress = tkd["contractAddress"].ToString();
                    tk.decimals = tkd["decimals"].ToString();
                    tk.name = tkd["name"].ToString();
                    tk.symbol = tkd["symbol"].ToString();
                    tk.type = tkd["type"].ToString();

                    SqliteCommand chk = SQLite.Query.SelectTokensByAddress(tk.contractAddress);
                    using (var reader = chk.ExecuteReader())
                    {
                        if (!reader.HasRows)
                        {
                            SQLite.Query.InsertToken(tk); // unchecked
                            API.Tokens.Add(tk);
                        }
                        else
                        {
                            API.Tokens.Add(tk);
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

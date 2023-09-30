using Microsoft.Data.Sqlite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Timers;

namespace Pulse
{
    internal class API
    {
        private static System.Timers.Timer rateLimitingTimer = null;
        public static List<API.Token> Tokens = null;

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
                while (rateLimitingTimer.Enabled == true) System.Threading.Thread.Sleep(100);
            else { 
                rateLimitingTimer = new System.Timers.Timer(1200);
                rateLimitingTimer.Elapsed += RateEndEvent;
                rateLimitingTimer.AutoReset = false;
                rateLimitingTimer.Enabled = false;
            }
            if (rateLimitingTimer.Enabled == false) rateLimitingTimer.Enabled = true;
        }

        private static void RateEndEvent(Object src, ElapsedEventArgs e)
        {
            rateLimitingTimer.Stop();
            rateLimitingTimer.Enabled = false;
            return;
        }

        private static List<Dictionary<string, string>> GetURI(Uri uri)
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
                    return SanitizeResult(s.Split(','));
                }
                catch (Exception ex)
                {
                    client.Dispose();
                    int e = 44;
                }
            }
        }

        public static List<Dictionary<string, string>> GetToken(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=token&action=getToken&contractaddress={0}&page=1&offset=100", ContractAddress)));
        }

        public static List<Dictionary<string, string>> GetTokenList(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", ContractAddress)));
        }

        public static List<Dictionary<string, string>> GetTokenHolders(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=token&action=getTokenHolders&contractaddress={0}&page=1&offset=100", ContractAddress)));
        }

        public static List<Dictionary<string, string>> GetAccountHoldings(string ContractAddress)
        {
            return GetURI(new Uri(String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", ContractAddress)));
        }

        public static List<Dictionary<string, string>> SanitizeResult(string[] s)
        {
            if (s[0] != "{\"message\":\"OK\"") throw new Exception("Doesn't Look OK");
            string[] s1chk = s[1].Split('\"');
            if (s1chk[1] != "result") throw new Exception("Doesn't Look Result !");

            List<Dictionary<string, string>> R = new List<Dictionary<string, string>>();
            Dictionary<string, string> T = new Dictionary<string, string>();
            T[s1chk[3]] = s1chk[5];

            int s3chk = 0;
            for (int i = 2; i < s.Count(); i++)
            {
                string[] s2chk = s[i].Split('\"');
                if (s2chk.Count() != 5) throw new Exception("Doesn't Look Check !");
                if (s2chk[0] == "{") {
                    R.Add(T);
                    if (s3chk == 0) s3chk = T.Count();
                    if (s3chk != T.Count) throw new Exception("Doesn't Look Count !");
                    T = new Dictionary<string, string>();
                }
                T[s2chk[1]] = s2chk[3];
            }

            return R;
        }

        public static void GetTokenDatas()
        {
            foreach (API.Token tk in API.Tokens)
            {
                int a = 44;
                if (tk.symbol != "PLP") continue;
                List<Dictionary<string, string>> t = API.GetAccountHoldings(tk.contractAddress);
                int v = 99;

            }
        }

        public static void GetTokens(String ContractAddress)
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

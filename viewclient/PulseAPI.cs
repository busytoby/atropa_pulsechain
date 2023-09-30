using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace Pulse
{
    internal class API
    {
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

        public static List<Dictionary<string, string>> GetToken(string ContractAddress)
        {
            HttpClient client = new HttpClient();
            Task<string> ts = client.GetStringAsync(new Uri(
                String.Format("https://scan.pulsechain.com/api?module=token&action=getToken&contractaddress={0}&page=1&offset=100", ContractAddress)));
            ts.Wait();
            client.Dispose();
            string s = ts.Result;
            return SanitizeResult(s.Split(','));
        }

        public static List<Dictionary<string, string>> GetTokenList(string ContractAddress)
        {
            HttpClient client = new HttpClient();
            Task<string> ts = client.GetStringAsync(new Uri(
                    String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", ContractAddress)));
            ts.Wait();
            client.Dispose();
            string s = ts.Result;
            return SanitizeResult(s.Split(','));
        }

        public static List<Dictionary<string, string>> GetTokenHolders(string ContractAddress)
        {
            HttpClient client = new HttpClient();
            Task<string> ts = client.GetStringAsync(new Uri(
                    String.Format("https://scan.pulsechain.com/api?module=token&action=getTokenHolders&contractaddress={0}&page=1&offset=100", ContractAddress)));
            ts.Wait();
            client.Dispose();
            string s = ts.Result;
            return SanitizeResult(s.Split(','));
        }

        public static List<Dictionary<string, string>> GetAccountHoldings(string ContractAddress)
        {
            HttpClient client = new HttpClient();
            Task<string> ts = client.GetStringAsync(new Uri(
                    String.Format("https://scan.pulsechain.com/api?module=account&action=tokenlist&address={0}", ContractAddress)));
            ts.Wait();
            client.Dispose();
            string s = ts.Result;
            return SanitizeResult(s.Split(','));
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
    }
}

using Microsoft.Data.Sqlite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace Pulse
{
    internal class SQLite
    {
        public class Query
        {
            public static string dbpath = "sqlite.db";
            public static SqliteConnection db = null;

            public Query()
            {
                if (db == null)
                {
                    db = new SqliteConnection($"Filename={dbpath}");
                    db.Open();
                }
            }

            public static int InsertToken(API.Token tk)
            {
                SqliteCommand ins = new SqliteCommand(String.Format("INSERT INTO Tokens (Address, Symbol, Name, Balance, Decimals, Type) VALUES (\"{0}\", \"{1}\", \"{2}\", \"{3}\", \"{4}\", \"{5}\");",
                                tk.contractAddress, tk.symbol, tk.name, tk.balance, tk.decimals, tk.type), db);
                return ins.ExecuteNonQuery();
            }

            public static int InsertContractHoldings(String ContractAddress, String Token, String Balance)
            {
                SqliteCommand ins = new SqliteCommand(String.Format("INSERT INTO ContractHoldings (Id, HolderContract, Asset, Balance) VALUES (\"{0}\", \"{1}\", \"{2}\", \"{3}\");",
                                System.Guid.NewGuid(), ContractAddress, Token, Balance), db);
                return ins.ExecuteNonQuery();
            }

            public static SqliteCommand SelectTokensByAddress(String ContractAddress)
            {
                return new SqliteCommand(String.Format("Select * From Tokens Where Address = \"{0}\";", ContractAddress), db);
            }

            public static SqliteCommand SelectAsset(String ContractAddress, String Token)
            {
                return new SqliteCommand(String.Format("Select * From ContractHoldings Where HolderContract = \"{0}\" AND Asset = \"{1}\";", ContractAddress, Token), db);
            }
        }
    }
}

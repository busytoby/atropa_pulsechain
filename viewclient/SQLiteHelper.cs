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
                    InitDB();
                }
            }

            public static void InitDB()
            {
                string tableCommand = "CREATE TABLE IF NOT EXISTS " +
                    "Tokens (Address NVARCHAR(256) PRIMARY KEY, " +
                    "Symbol NVARCHAR(256) NULL," +
                    "Name NVARCHAR(256) NULL," +
                    "Balance NVARCHAR(256) NULL," +
                    "Decimals NVARCHAR(8) NULL," +
                    "Type NVARCHAR(256) NULL)";

                var createTable = new SqliteCommand(tableCommand, db);
                createTable.ExecuteReader();
                createTable.Dispose();

                tableCommand = "CREATE TABLE IF NOT EXISTS " +
                    "Aliases (Address NVARCHAR(256) PRIMARY KEY," +
                    "Alias NVARCHAR(256) NULL)";
                createTable = new SqliteCommand(tableCommand, db);
                createTable.ExecuteReader();
                createTable.Dispose();


                tableCommand = "CREATE TABLE IF NOT EXISTS " +
                    "ContractHoldings (Id NVARCHAR(256) PRIMARY KEY," +
                    "HolderContract NVARCHAR(256) NULL," +
                    "Asset NVARCHAR(256) NULL," +
                    "Balance NVARCHAR(256) NULL," +
                    "UNIQUE(HolderContract, Asset))";
                createTable = new SqliteCommand(tableCommand, db);
                createTable.ExecuteReader();
                createTable.Dispose();
            }

            public static int InsertAlias(API.Token tk, String Alias)
            {
                SqliteCommand ins = new SqliteCommand(String.Format("INSERT INTO Aliases (Address, Alias) VALUES (\"{0}\", \"{1}\");",
                                tk.contractAddress, Alias), db);
                return ins.ExecuteNonQuery();
            }

            public static Dictionary<string, string> GetAliases()
            {
                Dictionary<string, string> Aliases = new Dictionary<string, string>();
                SqliteCommand chk = new SqliteCommand(String.Format("Select * From Aliases"), db);
                using (var reader = chk.ExecuteReader())
                    if (reader.HasRows)
                    {
                        while (reader.Read())
                        {
                            Aliases.Add(reader.GetString(0), reader.GetString(1));
                        }
                    }
                return Aliases;
            }

            public static string GetAlias(string ContractAddress)
            {
                string Alias = "";
                SqliteCommand chk = new SqliteCommand(String.Format("Select * From Aliases Where Address = \"{0}\";", ContractAddress), db);
                using (var reader = chk.ExecuteReader())
                    if (reader.HasRows)
                    {
                        reader.Read();
                        Alias = reader.GetString(1);
                    }
                chk.Dispose();
                return Alias;
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

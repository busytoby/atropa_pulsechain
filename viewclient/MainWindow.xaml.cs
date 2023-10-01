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
        /* reimplement in WPF AtropaWin later
        private void StageUI()
        {
            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.BeginInvoke(() => { StageUI(); });
                return;
            }

            int offset = 0;
            while (UIStage == 0)
            {
                while (API.Tokens.Count == 0 || API.Tokens.Count == sp.Children.Count && UIStage != 1) System.Threading.Thread.Sleep(1000);
                offset = PopulateSP(offset);
            }
            int i = 99;
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
        */
    }
}
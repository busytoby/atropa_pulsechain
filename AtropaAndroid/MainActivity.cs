namespace AtropaAndroid
{
    [Activity(Label = "@string/app_name", MainLauncher = true)]
    public class MainActivity : Activity
    {
        protected override void OnCreate(Bundle? savedInstanceState)
        {
            base.OnCreate(savedInstanceState);

            InitAPI();

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.activity_main);
        }

        /*
        public int PopulateSP(int offset = 0)
        {
            if (Pulse.API.UIUpdating) return offset;
            UIUpdating = true;

            int i = offset;
            for (; i < API.Tokens.Count; i++)
            {
                Pulse.API.Token tk = Pulse.API.Tokens[i];
                Border B = new Border();
                B.Background = Brushes.White;
                B.BorderBrush = Brushes.Thistle;
                B.BorderThickness = new Thickness(1);
                TextBlock T = new TextBlock();
                T.Foreground = Brushes.Black;
                T.FontSize = 16;
                String DisplayName = tk.name;
                if (API.Aliases.ContainsKey(tk.contractAddress))
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

        public void InitAPI()
        {
            Pulse.API Comptroller = new Pulse.API();
        }
    }
}
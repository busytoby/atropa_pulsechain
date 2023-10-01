using Android.Views;

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

            StartThreads();
        }

        private void StartThreads()
        {
            Action su = new Action(() => { StageUI(); });
            Task t3 = Task.Run(su);
        }

        private void StageUI()
        {
            int offset = 0;
            while (Pulse.API.UIStage == 0)
            {
                while (Pulse.API.Tokens.Count == 0 || Pulse.API.UIStage != 1) System.Threading.Thread.Sleep(1000);
                offset = PopulateSP(offset);
            }
            int i = 99;
        }

        public int PopulateSP(int offset = 0)
        {
            if (Pulse.API.UIUpdating) return offset;
            Pulse.API.UIUpdating = true;
            //LinearLayout linearLayout = new LinearLayout(this);
            LayoutInflater factory = (LayoutInflater)Application.Context.GetSystemService(LayoutInflaterService);
            View mView = factory.Inflate(Resource.Layout.activity_main, null);

            int i = offset;
            for (; i < Pulse.API.Tokens.Count; i++)
            {
                Pulse.API.Token tk = Pulse.API.Tokens[i];
                String DisplayName = tk.name;
                if (Pulse.API.Aliases.ContainsKey(tk.contractAddress))
                    DisplayName = Pulse.API.Aliases[tk.contractAddress];
                string v = $"{tk.contractAddress}\t{DisplayName} ({tk.symbol})\t{tk.balance}";
                TextView t = new TextView(this);
                t.SetText(v.ToCharArray(), 0, v.Length);
                RelativeLayout rl = (RelativeLayout)mView.FindViewById<RelativeLayout>(Resource.Id.rl);
                TextView tv = (TextView)mView.FindViewById<TextView>(Resource.Id.tv);
                RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WrapContent, RelativeLayout.LayoutParams.FillParent);
                lp.AddRule(LayoutRules.Below, tv.Id);
                rl.AddView(t, lp);
            }
            //SetContentView(linearLayout);
            Pulse.API.UIUpdating = false;
            return i;
        }

        public void InitAPI()
        {
            Pulse.API Comptroller = new Pulse.API();
        }
    }
}
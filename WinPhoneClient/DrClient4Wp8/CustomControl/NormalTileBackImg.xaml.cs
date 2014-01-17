using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using System.Globalization;
using System.Threading;

namespace CustomControl
{
    public partial class NormalTileBackImg : UserControl
    {
        private string ConnectStatusTxt;
        private string TileContentTxt;
        public NormalTileBackImg()
        {
            InitializeComponent();
            CultureInfo currentCul = Thread.CurrentThread.CurrentUICulture;
            switch (currentCul.Name)
            {
                case "en-US":
                case "en-GB":
                    TileContentTxt = Resource.TileContent_Disconn_EN;
                    ConnectStatusTxt = Resource.ConnectState_EN;
                    break;
                // 繁体中文
                case "zh-HK":
                case "zh-TW":
                case "zh-MO":
                    TileContentTxt = Resource.TileContent_Disconn_HK;
                    ConnectStatusTxt = Resource.ConnectState_HK;
                    break;

                default:
                    TileContentTxt = Resource.TileContent_Disconn_CN;
                    ConnectStatusTxt = Resource.ConnectState_CN;
                    break;
            }
            
        }
        public string GetConnStatusTxt()
        {
            return ConnectStatusTxt;
        }
        public string GetTileContentTxt()
        {
            return TileContentTxt;
        }
    }
}

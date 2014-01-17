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
    public partial class TileBackImg : UserControl
    {
        public TileBackImg()
        {
            InitializeComponent();
            CultureInfo currentCul = Thread.CurrentThread.CurrentUICulture;
            switch (currentCul.Name)
            {
                case "en-US":
                case "en-GB":
                    this.usedTimeTxt.Text = Resource.Used_time_EN;
                    this.usedFluxTxt.Text = Resource.Used_flux_EN;
                    this.min.Text = Resource.Min_EN;
                    this.Mbyte.Text = Resource.MByte_EN;
                    this.lastUpdateTimeTxt.Text = Resource.lastUpdateTime_EN;
                    break;
                    // 繁体中文
                case "zh-HK":
                case "zh-TW":
                case "zh-MO":

                    this.usedTimeTxt.Text = Resource.Used_time_HK;
                    this.usedFluxTxt.Text = Resource.Used_flux_HK;
                    this.min.Text = Resource.Min_HK;
                    this.Mbyte.Text = Resource.MByte_HK;
                    this.lastUpdateTimeTxt.Text = Resource.lastUpdateTime_HK;
                    break;

                default:

                    this.usedTimeTxt.Text = Resource.Used_time_CN;
                    this.usedFluxTxt.Text = Resource.Used_flux_CN;
                    this.min.Text = Resource.Min_CN;
                    this.Mbyte.Text = Resource.MByte_CN;
                    this.lastUpdateTimeTxt.Text = Resource.lastUpdateTime_CN;
                    break; 
            }
            
        }
        public void SetusedTime(string strTimeValue)
        {
   
            this.usedTimeValue.Text = strTimeValue;
   
        }
        public void SetusedFlux(string strFluxValue)
        {
   
            this.usedFluxValue.Text = strFluxValue;
   
        }
        public void SetLastUpdateTime(string strupdateTimeValue)
        {
       
            this.lastUpdateTimeValue.Text = strupdateTimeValue;
        }
    }
}

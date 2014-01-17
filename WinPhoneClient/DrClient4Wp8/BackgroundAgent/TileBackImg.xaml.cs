using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;

namespace DrClient4Wp8
{
    public partial class TileBackImg : UserControl
    {
        public TileBackImg()
        {
            InitializeComponent();
        }
        public void SetUsedTimeValue(string strusedTime)
        {
            this.usedTimeValue.Text = strusedTime;
        }
        public void SetUsedFluxValue(string strusedFlux)
        {
            this.usedFluxValue.Text = strusedFlux;
        }
        public void SetUpdateTime(string strupdateTime)
        {
            this.lastUpdateTime.Text = strupdateTime;
        }
    }
}

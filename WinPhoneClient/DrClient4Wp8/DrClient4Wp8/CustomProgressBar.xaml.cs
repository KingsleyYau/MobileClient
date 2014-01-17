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
    public partial class CustomProgressBar : UserControl
    {
        public CustomProgressBar()
        {
            InitializeComponent();
        }
        public void Show(string strContent)
        { 
           Content.Text = strContent;
           ProgressPopup.IsOpen = true;
        }
        public void Hide()
        {
            ProgressPopup.IsOpen = false;
        }
    }
}

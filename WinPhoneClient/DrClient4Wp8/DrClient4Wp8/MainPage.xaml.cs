//#define DEBUG_AGENT
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using System.IO.IsolatedStorage;
using DrClient4Wp8.Resources;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.ComponentModel;
using Microsoft.Phone.Info;
using Windows.Phone.System.Analytics;
using System.Windows.Threading;
using System.Windows.Media;
using Microsoft.Phone.Net.NetworkInformation;
using Microsoft.Phone.Tasks;
using Microsoft.Phone.Scheduler;
using System.Xml.Linq;
using System.Reflection;
using System.Windows.Media.Imaging;
using System.Diagnostics;
using DrClientDll;
//using CustomControl;
namespace DrClient4Wp8
{
    public partial class MainPage : PhoneApplicationPage
    {
        private DrClient _drClient = new DrClient();
        private AppSettings settings = new AppSettings();
        private BackgroundWorker _LoginThread = new BackgroundWorker();                 // 登录
        private BackgroundWorker _LogoutThread = new BackgroundWorker();              // 注销
        private BackgroundWorker _GetStatusThread = new BackgroundWorker();          // 获取状态
    //    private BackgroundWorker _GetUpdateInfoThread = new BackgroundWorker();    // 获取升级信息
     //   private BackgroundWorker _DetectLoginStatusThread = new BackgroundWorker();  // 检测登录状态
        private string strUserName;
        private string strPassWord;
        private string strGatewayAddr;
        private DispatcherTimer timerGetStatus;         // 获取状态的定时器
        private CustomProgressBar custProgressBar = new CustomProgressBar();          // 自定义进度条控件


        // 动态控件
        private TextBlock netFlow_static;               // 流量(文本)
        private TextBlock netFlow_value;               // 流量(值)
        private TextBlock netFlow_suffix;              // 流量(单位)
        private TextBlock netDuration_static;       // 时长(文本)
        private TextBlock netDuration_value;       // 时长(值)
        private TextBlock netDuration_suffix;      // 时长(单位)
        // -----------------
        public static bool bIsConnected;
        public static MainPage theInstance = null;

       // 后台代理任务
        PeriodicTask periodicTask;
        string periodicTaskName = "DrComAgent";
        public bool agentsAreEnabled = true;

        // 字符串常量      
        private const string storeUrl = "http://www.windowsphone.com/s?appid=";             // 
        private const string AppID = "be692f61-9665-4335-922c-72c93c830eb6";            // DrComWS GUID
        private const string versionElement = "<span itemprop=\"softwareVersion\">";
        private const string spanEnd = "</span>";  
      
        // 构造函数
        public MainPage()
        {
            theInstance = this;
            InitializeComponent();
            userName.MaxLength = 16;
            passWord.MaxLength = 16;
            strUserName = "";
            strPassWord = "";
            strGatewayAddr = "";
            bIsConnected =false;
            userName.Text = settings.UsernameSetting;
            strGatewayAddr = settings.GatewayAddrSetting;
            cbSavePassword.IsChecked = settings.SavePasswordSetting;
            if (cbSavePassword.IsChecked == true)
            {
                // 有勾选保存密码
                passWord.Password = settings.PasswordSetting;
                cbAutoLogin.IsChecked = settings.AutoLogindSetting;
            }
            
            InitDynamicControls();
  
            // 定时器
            timerGetStatus = new DispatcherTimer();
            timerGetStatus.Tick += new EventHandler(time_Tick);
            timerGetStatus.Interval = new System.TimeSpan(0,0,30);
            
            // 后台获取状态的线程事件
            _GetStatusThread.DoWork += new DoWorkEventHandler(GetStatus_DoWork);
            _GetStatusThread.RunWorkerCompleted += new RunWorkerCompletedEventHandler(GetStatus_RunWorkercompleted);

            // 后台拨号线程订阅事件
            _LoginThread.DoWork += new DoWorkEventHandler(loginTh_DoWork);
            _LoginThread.RunWorkerCompleted += new RunWorkerCompletedEventHandler(loginTh_RunWorkercompleted);

            // 后台注销线程订阅事件
            _LogoutThread.DoWork += new DoWorkEventHandler(logoutTh_DoWork);
            _LogoutThread.RunWorkerCompleted += new RunWorkerCompletedEventHandler(LogoutTh_RunWorkercompleted);

            _drClient.Init();

            object uniqueId = DeviceExtendedProperties.GetValue("DeviceUniqueId");
            if (uniqueId != null)
            {
                byte[] uniqueBytes = (byte[])uniqueId;

                _drClient.SetDeviceID(uniqueBytes);
           
            }

            // 用于本地化 ApplicationBar 的示例代码
            //BuildLocalizedApplicationBar();
        }
        private void RemoveAgent(string name)
        {
            try
            {
                ScheduledActionService.Remove(name);
            }
            catch (Exception)
            {

            }
        }
        // 开启后台代理
        private void StartPeriodicAgent()
        {
            agentsAreEnabled = true;
            periodicTask = ScheduledActionService.Find(periodicTaskName) as PeriodicTask;

            if (periodicTask != null)
            {
                RemoveAgent(periodicTaskName);
            }
            periodicTask = new PeriodicTask(periodicTaskName);

            periodicTask.Description = "DR.Com后台任务，用于更新磁贴";

            // 添加后台任务代理
            #region 尝试添加后台任务代理
            try
            {
                ScheduledActionService.Add(periodicTask);

                // If debugging is enabled, launch the agent again in one minute.
#if DEBUG_AGENT
                ScheduledActionService.LaunchForTest(periodicTaskName, TimeSpan.FromSeconds(60));
#endif

            }
            catch (InvalidOperationException exception)
            {
                if (exception.Message.Contains("BNS Error: The action is disabled"))
                {
                    MessageBox.Show("Background agents for this application have been disabled by the user.");
                    agentsAreEnabled = false;
                }
                if (exception.Message.Contains("BNS Error: The maximum number of ScheduledActions of this type have already been added."))
                {
                    agentsAreEnabled = false;
                }
            }
            catch (SchedulerServiceException )
            {
            }
            #endregion
        }

        public static bool GetStatus()
        {
            if (theInstance != null && bIsConnected)
            {
                theInstance.timerGetStatus.Stop();
                theInstance.time_Tick(new object(), new EventArgs());
                theInstance.timerGetStatus.Start();
                return true;
            }
            else
            {
                return false;
            }
        }

        public void UpdateTileState()
        {
            const string filename = "/Shared/ShellContent/NormalTileBack.jpg";
            // 看默认的背景图是否已经存在
            using (var isf = IsolatedStorageFile.GetUserStoreForApplication())
            {

           //     if (!isf.DirectoryExists("/CustomLiveTiles"))
              //  {
                 //   isf.CreateDirectory("/CustomLiveTiles");
                //}
                if (isf.FileExists(filename))
                {
                    isf.DeleteFile(filename);
                }
                    // 不存在则创建一个新的
                    NormalTileBackImg normalImg = new NormalTileBackImg();
               //     normalImg.Measure(new Size(336, 336));
                    normalImg.Arrange(new Rect(0, 0, 336, 336));

                    var bmp = new WriteableBitmap(336, 336);
                    bmp.Render(normalImg, null);
                    bmp.Invalidate();

                    using (var stream = isf.OpenFile(filename, System.IO.FileMode.OpenOrCreate))
                    {
                        bmp.SaveJpeg(stream, 336, 366, 0, 100);
                    }
       
                try
                {
                    ShellTile TileToFind = ShellTile.ActiveTiles.First();
                    FlipTileData tileData = new FlipTileData
                    {
                        BackTitle = AppResources.ConnectState,
                        BackContent=AppResources.TileContent_Disconn,
                        BackBackgroundImage = new Uri("isostore:" + filename, UriKind.Absolute),
                    };
                    TileToFind.Update(tileData);
                }
                catch (Exception e)
                {
                    MessageBox.Show(e.Message);
                }

            }

        }

        // 已经登录成功的情况下更新磁贴状态
        public void UpdateTile(string usedTime, string usedFlux, string updateTime)
        {
            var customTile = new TileBackImg();
            customTile.SetUsedTimeValue(usedTime);
            customTile.SetUsedFluxValue(usedFlux);
            customTile.SetUpdateTime(updateTime);
            //customTile.Measure(new Size(336, 336));
            customTile.Arrange(new Rect(0, 0, 336, 336));

            var bmp = new WriteableBitmap(336, 336);
            bmp.Render(customTile, null);
            bmp.Invalidate();

            const string filename = "/Shared/ShellContent/CustomTile.jpg";

            using (var isf = IsolatedStorageFile.GetUserStoreForApplication())
            {
                if (!isf.DirectoryExists("/CustomLiveTiles"))
                {
                    isf.CreateDirectory("/CustomLiveTiles");
                }
                if (isf.FileExists(filename))
                {
                    isf.DeleteFile(filename);
                }

                using (var stream = isf.OpenFile(filename, System.IO.FileMode.OpenOrCreate))
                {
                    bmp.SaveJpeg(stream, 336, 366, 0, 100);
                }
            }
            try
            {
                ShellTile TileToFind = ShellTile.ActiveTiles.First();
                FlipTileData tileData = new FlipTileData
                {
                    BackTitle = "",
                    BackContent="",
                    BackBackgroundImage = new Uri("isostore:" + filename, UriKind.Absolute),
                };

                TileToFind.Update(tileData);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }
        // 定时器
        private void time_Tick(object sender, EventArgs e)
        {
            if( bIsConnected)
            {
                if (_GetStatusThread.IsBusy == false)
                {
                    _GetStatusThread.RunWorkerAsync();
                } 
            }
        }
        // 初始化动态控件，只调用一次
        private void InitDynamicControls()
        {
            // netFlow_static 流量（文本）
            netFlow_static = new TextBlock();
            netFlow_static.Margin = new Thickness(10, 10, 5, 10);
            netFlow_static.HorizontalAlignment = HorizontalAlignment.Right;
            netFlow_static.VerticalAlignment =  VerticalAlignment.Center;
            netFlow_static.FontSize = 21.333;
            netFlow_static.Text = AppResources.Used_flux;
            Grid.SetRow(netFlow_static, 2);
            Grid.SetColumn(netFlow_static, 1);
            netFlow_static.Visibility = Visibility.Collapsed;
            ControlPanel.Children.Add(netFlow_static);
      
            // netFlow_value 流量（值）

            netFlow_value = new TextBlock();
            netFlow_value.Margin = new Thickness(20, 3, 0, 0);
            netFlow_value.HorizontalAlignment = HorizontalAlignment.Left;
            netFlow_value.VerticalAlignment =  VerticalAlignment.Center;
            netFlow_value.TextAlignment = TextAlignment.Center;
            netFlow_value.FontSize = 24;
            netFlow_value.Foreground = new SolidColorBrush(Colors.Blue);
        //    netFlow_value.Height = 65;
         //   netFlow_value.
            Grid.SetRow(netFlow_value, 2);
            Grid.SetColumn(netFlow_value, 2);
            netFlow_value.Visibility = Visibility.Collapsed;
            ControlPanel.Children.Add(netFlow_value);

            // netFlow_suffix 流量（单位）
            netFlow_suffix = new TextBlock();
            netFlow_suffix.Margin = new Thickness(0, 3, 0, 0);
            netFlow_suffix.MinWidth = 80;
            netFlow_suffix.HorizontalAlignment = HorizontalAlignment.Left;
            netFlow_suffix.VerticalAlignment = VerticalAlignment.Center;
            netFlow_suffix.TextAlignment = TextAlignment.Right;
            netFlow_suffix.FontSize = 21.333;
            netFlow_suffix.Text = AppResources.MByte;
            Grid.SetRow(netFlow_suffix, 2);
            Grid.SetColumn(netFlow_suffix, 3); 
            netFlow_suffix.Visibility = Visibility.Collapsed;
            ControlPanel.Children.Add(netFlow_suffix);

            //netDuration_static 时长（文本）
            netDuration_static = new TextBlock();
            netDuration_static.Margin = new Thickness(10, 10, 5, 10);
            netDuration_static.HorizontalAlignment = HorizontalAlignment.Right;
            netDuration_static.VerticalAlignment =  VerticalAlignment.Center;
            netDuration_static.FontSize = 21.333;
            netDuration_static.Text = AppResources.Used_time;
            Grid.SetRow(netDuration_static, 1);
            Grid.SetColumn(netDuration_static, 1);
            netDuration_static.Visibility = Visibility.Collapsed;
            ControlPanel.Children.Add(netDuration_static);
 
            // netDuration_value 时长（值）
            netDuration_value = new TextBlock();
            netDuration_value.Margin = new Thickness(20,3,0,0);
            netDuration_value.HorizontalAlignment = HorizontalAlignment.Left;
            netDuration_value.VerticalAlignment =  VerticalAlignment.Center;
            netDuration_value.TextAlignment = TextAlignment.Center;
            netDuration_value.FontSize = 24;
            netDuration_value.Foreground = new SolidColorBrush(Colors.Blue);
           // netDuration_value.Height = 65;
            Grid.SetRow(netDuration_value, 1);
            Grid.SetColumn(netDuration_value, 2);
            netDuration_value.Visibility = Visibility.Collapsed;
            ControlPanel.Children.Add(netDuration_value);

            //netDuration_suffix  时长(单位)
            netDuration_suffix = new TextBlock();
            netDuration_suffix.Margin = new Thickness(0, 3, 0, 0);
            netDuration_suffix.MinWidth = 80;
            netDuration_suffix.HorizontalAlignment = HorizontalAlignment.Left;
            netDuration_suffix.VerticalAlignment = VerticalAlignment.Center;
            netDuration_suffix.TextAlignment = TextAlignment.Right;

            netDuration_suffix.FontSize = 21.333;
            netDuration_suffix.Text = AppResources.Min;
            Grid.SetRow(netDuration_suffix, 1);
            Grid.SetColumn(netDuration_suffix, 3);
            netDuration_suffix.Visibility = Visibility.Collapsed;
            ControlPanel.Children.Add(netDuration_suffix);

        }

        // 动态调整控件排版
        private void DynamicChangeLayout()
        {
            if (bIsConnected)
            {
                // 当前已经连接上，处于登录状态

                // 隐藏用户名和密码控件，改用
                txtUsername.Visibility = Visibility.Collapsed;
                userName.Visibility = Visibility.Collapsed;
                txtPassword.Visibility = Visibility.Collapsed;
                passWord.Visibility = Visibility.Collapsed;
                ControlPanel.ColumnDefinitions[2].MinWidth = 140;

                // 显示动态控件
                netFlow_static.Visibility = Visibility.Visible;
                netFlow_value.Visibility = Visibility.Visible;
                netFlow_suffix.Visibility = Visibility.Visible;
                netDuration_static.Visibility = Visibility.Visible;
                netDuration_value.Visibility = Visibility.Visible;
                netDuration_suffix.Visibility = Visibility.Visible;
            } 
            else
            {
                // 当前未连接，处于注销状态

                // 显示用户名和密码控件，改用
                txtUsername.Visibility = Visibility.Visible;
                userName.Visibility = Visibility.Visible;
                txtPassword.Visibility = Visibility.Visible;
                passWord.Visibility = Visibility.Visible;

                // 隐藏动态控件
                netFlow_static.Visibility = Visibility.Collapsed;
                netFlow_value.Visibility = Visibility.Collapsed;
                netFlow_suffix.Visibility = Visibility.Collapsed;
                netDuration_static.Visibility = Visibility.Collapsed;
                netDuration_value.Visibility = Visibility.Collapsed;
                netDuration_suffix.Visibility = Visibility.Collapsed;

            }
        }

        // 获取状态
        private void GetStatus_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            if (DeviceNetworkInformation.IsWiFiEnabled == false)
            {
                e.Result = ClientHandleStatus.DrClient_NetworkError;
            }
            else
            {
                e.Result = _drClient.GetStatus();
            }

        }
        private void GetStatus_RunWorkercompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if ((ClientHandleStatus)e.Result == ClientHandleStatus.DrClient_Success)
            {
            
                // 获取成功
               string strTime = _drClient.GetTimeStatus();
               string strFlux = _drClient.GetFluxStatus();
               string strupdateTime =   DateTime.Now.ToString("HH:mm");
               UpdateTile(strTime, strFlux,strupdateTime);

               netDuration_value.Text = strTime;
               netFlow_value.Text = strFlux;

            }
            else
            { 
                // 获取失败，设备可能不在WIFI范围内或者网络已断开
                timerGetStatus.Stop();

                //更新磁贴
                UpdateTileState();

                MessageBox.Show(AppResources.DrClient_NetworkError, "", MessageBoxButton.OK);
                bIsConnected = false;
           
                btnLogin.Content = AppResources.btnLogin;
                btnLogin.Style = (Style)this.Resources["ButtonStyle1"];
                DynamicChangeLayout();
                cbAutoLogin.IsEnabled = true;
                cbSavePassword.IsEnabled = true;

             
                //// 更新磁贴
                //ShellTile TileToFind = ShellTile.ActiveTiles.First();
                //if (TileToFind != null)
                //{
                //    string strBackContent = "未登录或已断开连接";

                //    // 更新磁贴
                //    StandardTileData NewTileDta = new StandardTileData
                //    {

                //        //    BackBackgroundImage = new Uri("../DrClient4Wp8/Resources/App.png", UriKind.Relative),
                //        Count = 0,
                //        BackTitle = "连接状态",
                //        //      BackgroundImage = new Uri()
                //        BackContent = strBackContent
                //    };
                //    // 更新
                //    TileToFind.Update(NewTileDta);
                //}
              
            }
        }


        // 获取升级信息 函数
        private void GetUpdateInfo()
        {
            try
            {
                Uri requestUri = new Uri(storeUrl + AppID, UriKind.Absolute);
                // Create a HttpWebrequest object to the desired URL.
                HttpWebRequest httpRequest = (HttpWebRequest)HttpWebRequest.Create(requestUri);
                httpRequest.BeginGetResponse(httpRequestCallback, httpRequest);
            }
            catch(WebException webEx)
            {
                Debug.WriteLine(webEx.Message);
            }
            catch (Exception Ex)
            {
                Debug.WriteLine(Ex.Message);
            }
        }

        // http回调函数
        private  void httpRequestCallback(IAsyncResult result)
        {
            // 获取程序集的信息
            AssemblyName assemblyName = new AssemblyName(Assembly.GetExecutingAssembly().FullName);
            try
            {
                // 获取返回状态
                HttpWebRequest request = (HttpWebRequest)result.AsyncState;
                using (WebResponse responsce = request.EndGetResponse(result))
                {
                    using (StreamReader reader = new StreamReader(responsce.GetResponseStream()))
                    {
                        string htmlString = reader.ReadToEnd();             // 读取页面的内容
                        // 提取应用最新的版本号
                        int start = htmlString.IndexOf(versionElement) + versionElement.Length;
                        int end = htmlString.IndexOf(spanEnd, start);
                        if (start >= 0 && end > start)
                        {
                            string versionString = htmlString.Substring(start, end - start);
                            Version storeVersion = null;
                            MessageBoxResult msgRet;
                            Dispatcher.BeginInvoke(() =>
                            {
                                if (Version.TryParse(versionString, out storeVersion))
                                {
                                    // 字符串解析版本成功
                                    if (storeVersion > assemblyName.Version)
                                    {
                                        msgRet = MessageBox.Show(AppResources.Upgrade_msg, AppResources.UpgradeTips, MessageBoxButton.OKCancel);
                                        if (msgRet == MessageBoxResult.OK)
                                        {
                                            MarketplaceDetailTask storeTask = new MarketplaceDetailTask();
                                            storeTask.ContentIdentifier = AppID;
                                            storeTask.ContentType = MarketplaceContentType.Applications;
                                            storeTask.Show();
                                        }
                                    }
                                }
                            });
                        }
                    }
                }
            }
            catch (Exception eex)
            {
                Debug.WriteLine(eex.Message);
            }
        }

        // 开始拨号
        private void loginTh_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            //if (settings.GatewayAddrSetting == "")
            //{
            //    strGatewayAddr = "";
            //}
            //else
            //{
            //    strGatewayAddr = settings.GatewayAddrSetting;
            //}
            strGatewayAddr = settings.GatewayAddrSetting;
            e.Result = _drClient.Login(strGatewayAddr,strUserName, strPassWord);
            
            // 如果在拨号过程中
            if (bw.CancellationPending)
            {
                e.Cancel = true;
            }

        }
        // 拨号完成
        private void loginTh_RunWorkercompleted(object sender, RunWorkerCompletedEventArgs e) 
        {
            custProgressBar.Hide();
            string errorMessage="";
            switch ((ClientHandleStatus)e.Result)
            { 
              // 进行判断
                case ClientHandleStatus.DrClient_Success:
                    {
                        // 拨号成功
                   
                        bIsConnected = true;
                    }break;
                case ClientHandleStatus.DrClient_AccountBreakOff:
                    {
                        errorMessage = AppResources.DrClient_AccountBreakOff;
                    }break;
                case ClientHandleStatus.DrClient_AccountNotAllow:
                    {
                        errorMessage = AppResources.DrClient_AccountNotAllow;
                    }break;
                case ClientHandleStatus.DrClient_AccountTieUp:
                    {
                        errorMessage = string.Format(AppResources.DrClient_AccountTieUp, _drClient.GetXip(), _drClient.GetMac());
                    }break;
                case ClientHandleStatus.DrClient_AlreadyOnline:
                    {
                        errorMessage = AppResources.DrClient_AlreadyOnline;
                    }break;
                case ClientHandleStatus.DrClient_ChargeOrFluxOver:
                    {
                        errorMessage = AppResources.DrClient_ChargeOrFluxOver;
                    }break;
                case ClientHandleStatus.DrClient_InvalidAccountOrPwd:
                    {
                        errorMessage = AppResources.DrClient_InvalidAccountOrPwd;
                    }break;
                case ClientHandleStatus.DrClient_IpNotAllowLogin:
                    {
                        errorMessage = AppResources.DrClient_IpNotAllowLogin;
                    }break;
                case ClientHandleStatus.DrClient_NetworkError:
                    {
                        errorMessage = AppResources.DrClient_NetworkError;
                    }break;
                case ClientHandleStatus.DrClient_NewAndConfirmPwdDiffer:
                    {
                        errorMessage = AppResources.DrClient_NewAndConfirmPwdDiffer;
                    }break;
                case ClientHandleStatus.DrClient_NotAllowChangePwd:
                    {
                        errorMessage = AppResources.DrClient_NotAllowChangePwd;
                    }break;
                case ClientHandleStatus.DrClient_NotFoundDrCOM:
                    {
                        // 获取当前的网络SSID
                        bool bIsWlanConnected = false;
                        string CurrentSSID = GetCurrentSSID(ref bIsWlanConnected);
                        errorMessage = string.Format(AppResources.DrClient_NotFoundDrCOM, CurrentSSID);
                    }break;
                case ClientHandleStatus.DrClient_PwdAmendSuccessed:
                    {
                        errorMessage = AppResources.DrClient_PwdAmendSuccessed;
                    }break;
                case ClientHandleStatus.DrClient_SystemBufferFull:
                    {
                        errorMessage = AppResources.DrClient_SystemBufferFull;
                    }break;
                case ClientHandleStatus.DrClient_TieUpCannotAmend:
                    {
                        errorMessage = AppResources.DrClient_TieUpCannotAmend;
                    }break;
                case ClientHandleStatus.DrClient_UndefineError:
                    {
                        errorMessage = AppResources.DrClient_UndefineError;
                    }break;
                case ClientHandleStatus.DrClient_UsingAppointedMac:
                    {
                        errorMessage = string.Format(AppResources.DrClient_UsingAppointedMac, _drClient.GetMac());
                    }break;
                case ClientHandleStatus.DrClient_UsingNAT:
                    {
                        errorMessage = AppResources.DrClient_UsingNAT;
                    }break;
                case ClientHandleStatus.DrClient_UsingOnAppointedIp:
                    {
                        errorMessage = string.Format(AppResources.DrClient_UsingOnAppointedIp, _drClient.GetXip());          
                    }break;
                default:
                    {
                        errorMessage = AppResources.DrClient_UndefineError;
                    }break;
            }

            if (bIsConnected == true)
            {
                // 保存最后一次正确登陆的网关，用户名和密码
                settings.GatewayAddrSetting = _drClient.GetGatewayAddress();
                settings.UsernameSetting = strUserName;
                settings.PasswordSetting = strPassWord;
                // 2013-6-2 Added by Sanwen
                settings.SavePasswordSetting = cbSavePassword.IsChecked.Value;
                settings.AutoLogindSetting = cbAutoLogin.IsChecked.Value;

                // 2013-8-28 Added by Sanwen
                // 将网关写入文件中
                using (IsolatedStorageFile storage = IsolatedStorageFile.GetUserStoreForApplication())
                {
                    if (storage.FileExists("settings"))
                    {
                        storage.DeleteFile("settings");
                    }


                    try
                    {
                        XElement _item = new XElement("Settings");
                        _item.Add(new XElement("GatewayAddr", _drClient.GetGatewayAddress()));
                        _item.Add(new XElement("schedulingCount", 0));
                        // 用_item 新建一个XML的Linq文档
                        XDocument _doc = new XDocument(new XDeclaration("1.0", "utf-8", "yes"), _item);
                        // 创建一个独立存储的文件流
                        IsolatedStorageFileStream location = new IsolatedStorageFileStream("settings", System.IO.FileMode.OpenOrCreate, FileAccess.ReadWrite, storage);

                        // 将独立存储文件转化为可写流
                        System.IO.StreamWriter file = new System.IO.StreamWriter(location);
                        // 将XML文件保存到流file上， 即可以写入到手机独立存储文件上
                        _doc.Save(file);
                        file.Dispose();                               // 关闭可写流
                        location.Dispose();                       //关闭手机独立存储流
                    }
                    catch (Exception ex)
                    {
                        string strerr = ex.Message;
                    }

                }
                // ---End-tag---
                btnLogin.Content = AppResources.btnLogout;
                btnLogin.Style = (Style)this.Resources["ButtonStyle2"];

                // 禁止CheckBox
                cbAutoLogin.IsEnabled = false;
                cbSavePassword.IsEnabled = false;
                DynamicChangeLayout();
                time_Tick(new object(), new EventArgs());
                timerGetStatus.Start();
                // 登录成功
                GetUpdateInfo();
                
                // 开启后台代理
                StartPeriodicAgent();
            }
            else
            {
                // 登录失败
                btnLogin.Content = AppResources.btnLogin;
                // 提示用户失败原因
                MessageBox.Show(errorMessage, "", MessageBoxButton.OK);
            }

        }

        // 开始注销
        private void logoutTh_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            e.Result  = _drClient.Logout();

            //// 如果在注销过程
            //if (bw.CancellationPending)
            //{
            //    e.Cancel = true;
            //}
        }
        
        private void LogoutTh_RunWorkercompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            custProgressBar.Hide();
            string errorMessage="";
            switch ((ClientHandleStatus)e.Result)
            {
                // 进行判断
                case ClientHandleStatus.DrClient_Success:
                    {
                        // 注销成功
                        bIsConnected = false;
                    } break;
                case ClientHandleStatus.DrClient_NetworkError:
                    {
                        errorMessage = AppResources.DrClient_NetworkError;
                    } break;
                case ClientHandleStatus.DrClient_UndefineError:
                   {
                       errorMessage = AppResources.DrClient_UndefineError;
                    }break;
                default:
                    {
                        errorMessage = AppResources.DrClient_UndefineError;
                    }break;
            }

            if (bIsConnected == false)
            {
                //更新磁贴
               // UpdateTileState();

                btnLogin.Content = AppResources.btnLogin;
                btnLogin.Style = (Style)this.Resources["ButtonStyle1"];
                DynamicChangeLayout();
                // 禁止CheckBox
                cbAutoLogin.IsEnabled = true;
                cbSavePassword.IsEnabled = true;
                timerGetStatus.Stop();
                time_Tick(new object(), new EventArgs());

                // 注销成功，清空网关地址
                settings.GatewayAddrSetting = "";

                periodicTask = ScheduledActionService.Find(periodicTaskName) as PeriodicTask;

                if (periodicTask != null)
                {
                    RemoveAgent(periodicTaskName);
                }
                // 更新磁贴
                UpdateTileState();


            }
            else
            {
                // 注销失败
                btnLogin.Content = AppResources.btnLogout;
                // 提示用户失败原因
                MessageBox.Show(errorMessage, "", MessageBoxButton.OK);

                bIsConnected = false;
                btnLogin.Content = AppResources.btnLogin;
                btnLogin.Style = (Style)this.Resources["ButtonStyle1"];
                DynamicChangeLayout();
                // 禁止CheckBox
                cbAutoLogin.IsEnabled = true;
                cbSavePassword.IsEnabled = true;
                timerGetStatus.Stop();
                time_Tick(new object(), new EventArgs());

            }
        }

        private string GetCurrentSSID(ref bool bIsWifiConnected)
        {
             // 获取当前的网络SSID
            string CurrentSSID = "";
            
            foreach (var network in new NetworkInterfaceList())
            {
                string ssid = network.InterfaceName;
                if (
                    (network.InterfaceType == NetworkInterfaceType.Wireless80211) &&
                    (network.InterfaceState == ConnectState.Connected)
                    )
                {
                    bIsWifiConnected = true;
                    CurrentSSID = network.InterfaceName ;
                }
            }
            return CurrentSSID;
        }
 
        //// 开始检测登录状态
        //private void DetectTh_DoWork(object sender, DoWorkEventArgs e)
        //{
        //    e.Result = _drClient.GetStatus();
        //}
        //// 检测登录状态完成
        //private void DetectTh_RunWorkercompleted(object sender, RunWorkerCompletedEventArgs e)
        //{
        //    if ((ClientHandleStatus)e.Result == ClientHandleStatus.DrClient_Success)
        //    {
        //        // 获取成功，证明当前状态已登录
        //        _LoginThread.RunWorkerAsync();
                
        //    }
        //    else
        //    {
        //        // 获取失败，设备可能不在WIFI范围内或者网络已断开
        //        custProgressBar.Hide();
        //        MessageBox.Show(AppResources.DrClient_NetworkError, "", MessageBoxButton.OK);
        //        //bIsConnected = false;
        //        //btnLogin.Content = AppResources.btnLogin;
        //        //btnLogin.Style = (Style)this.Resources["ButtonStyle1"];
        //        //DynamicChangeLayout();
        //        //cbAutoLogin.IsEnabled = true;
        //        //cbSavePassword.IsEnabled = true;
        //        //timerGetStatus.Stop();
        //    }
        //}
        
        protected override void OnBackKeyPress(System.ComponentModel.CancelEventArgs e)
        {
        
            MessageBoxResult result = MessageBox.Show(AppResources.closingMsgTip,
                                                    AppResources.closingMsgTitle,
                                                    MessageBoxButton.OKCancel);

            if (result == MessageBoxResult.Cancel)
            {
                // 不退出
                e.Cancel = true;
            }
            else if (result == MessageBoxResult.OK)
            {
                // 真的确定退出
      
              //  settings.UsernameSetting = userName.Text.Trim();
                // 若当前是登录状态则先注销网络
                if (bIsConnected)
                {
                    //custProgressBar.Show(AppResources.logoutText);

                    if (0 == _drClient.Logout())
                    {
                        settings.GatewayAddrSetting = "";
                        bIsConnected = false;

                        periodicTask = ScheduledActionService.Find(periodicTaskName) as PeriodicTask;

                        if (periodicTask != null)
                        {
                            RemoveAgent(periodicTaskName);
                        }

                        UpdateTileState();
                        //// 更新磁贴
                        //ShellTile TileToFind = ShellTile.ActiveTiles.First();
                        //if (TileToFind != null)
                        //{
                        //    string strBackContent = "未登录或已断开连接";

                        //    // 更新磁贴
                        //    StandardTileData NewTileDta = new StandardTileData
                        //    {

                        //        //    BackBackgroundImage = new Uri("../DrClient4Wp8/Resources/App.png", UriKind.Relative),
                        //        Count = 0,
                        //        BackTitle = "连接状态",
                        //        //      BackgroundImage = new Uri()
                        //        BackContent = strBackContent
                        //    };
                        //    // 更新
                        //    TileToFind.Update(NewTileDta);
                        //}
                    }
                    //custProgressBar.Hide();
                }
              //  App.SaveCurrentStatus();     
               
                if ((settings.SavePasswordSetting == false) && (settings.GatewayAddrSetting.Length == 0))
                {
                    settings.PasswordSetting = "";
                }
                         
                base.OnBackKeyPress(e);
            }       
        }

        private void cbAutoLogin_Checked(object sender, RoutedEventArgs e)
        {
            cbSavePassword.IsChecked = true;
        }

       private  void btnLogin_Click(object sender, RoutedEventArgs e)
        {
            if (bIsConnected)
            {
                // 状态：已登陆，准备注销
                if (_LogoutThread.IsBusy != true)
                {
                    custProgressBar.Show(AppResources.logoutText);
                    //timerGetStatus.Stop();
                    //if (_GetStatusThread.IsBusy)
                    //{
                    //    _GetStatusThread.CancelAsync(); 
                    //}
                   //  UpdateTileState();
                    _LogoutThread.RunWorkerAsync();
                   
                }
            }
            else
            {
                //  状态：已注销， 准备登陆
                bool bIsWifiConnected = false;
                string ConnWifiSSID = GetCurrentSSID(ref bIsWifiConnected);
                if (bIsWifiConnected)
                {
                    // 已连接WLAN
                    strUserName = userName.Text.Trim();
                    strPassWord = passWord.Password.Trim();

                    // 检测用户名或密码是否为空，空则提示用户
                    if (strUserName.Length == 0 || strPassWord.Length == 0)
                    {
                        MessageBox.Show(AppResources.EmptyUsernameOrPassword, "", MessageBoxButton.OK);
                        return;
                    }
                    if (_LoginThread.IsBusy != true)
                    {
                        _drClient.SetWlanSSID(ConnWifiSSID);
                        _LoginThread.RunWorkerAsync();
                        custProgressBar.Show(AppResources.loggingText);
                    }

                }
                else
                {
                    // 未连接WLAN,提示用户
                    MessageBox.Show(AppResources.WlanNotConnected, "", MessageBoxButton.OK);
                    return;
                }
            }
        }
        
        private void cbSavePassword_Unchecked(object sender, RoutedEventArgs e)
        {
            cbAutoLogin.IsChecked = false;
        }
        private void LayoutRoot_Loaded(object sender, RoutedEventArgs e)
        {
            // 设置状态栏透明
            SystemTray.Opacity = 0;
            DeviceID.Text = _drClient.GetLocalMac();

            double actualHeight = LayoutRoot.ActualHeight;
            double actualWidth = LayoutRoot.ActualWidth;

            // 设置进度条控件的大小

            custProgressBar.LayoutRoot.Width = actualWidth;
            custProgressBar.LayoutRoot.Height = actualHeight + 32; // 必须加上32

            if( strGatewayAddr.Length == 0)
            {
                // 上一次有正常注销
                if (cbAutoLogin.IsChecked == true)
                {
                    // 用户勾选了自动登录
                    strUserName = userName.Text.Trim();
                    strPassWord = passWord.Password.Trim();
                    if (strUserName.Length > 0 && strPassWord.Length > 0)
                    {
                        btnLogin_Click(new object(), new RoutedEventArgs());
                    }
                }

            }
            else
            {
                // 上一次没有正常注销

                bool bIsWifiConnected = false;
                string ConnWifiSSID = GetCurrentSSID(ref bIsWifiConnected);
                if (bIsWifiConnected)
                {
                   // strUserName = settings.UsernameSetting;
                    // strPassWord = settings.PasswordSetting;
                    strUserName = userName.Text.Trim();
                    strPassWord = passWord.Password.Trim();
                    _drClient.SetGatewayAddress(strGatewayAddr);
                    _drClient.SetWlanSSID(ConnWifiSSID);
                    custProgressBar.Show(AppResources.loggingText);
                    _LoginThread.RunWorkerAsync();
                }
                else
                {
                    // 未连接WLAN,提示用户
                    MessageBox.Show(AppResources.WlanNotConnected, "", MessageBoxButton.OK);
                }
    
           //     custProgressBar.Show(AppResources.loggingText);
         //       _DetectLoginStatusThread.RunWorkerAsync();
                // 检测当前的登录状态

                //if ((ClientHandleStatus)_drClient.GetStatus() == ClientHandleStatus.DrClient_Success)
                //{
                //    // 获取状态成功，说明已经登录上
                //    btnLogin_Click(new object(), new RoutedEventArgs());
                //}
                //else
                //{
                //    // 获取状态失败，说明与网关已断开连接或者不在WIFI范围内
                //    MessageBox.Show(AppResources.DrClient_NetworkError, "", MessageBoxButton.OK);
                //}
            }

    
           
        }
  

        // 用于生成本地化 ApplicationBar 的示例代码
        //private void BuildLocalizedApplicationBar()
        //{
        //    // 将页面的 ApplicationBar 设置为 ApplicationBar 的新实例。
        //    ApplicationBar = new ApplicationBar();

        //    // 创建新按钮并将文本值设置为 AppResources 中的本地化字符串。
        //    ApplicationBarIconButton appBarButton = new ApplicationBarIconButton(new Uri("/Assets/AppBar/appbar.add.rest.png", UriKind.Relative));
        //    appBarButton.Text = AppResources.AppBarButtonText;
        //    ApplicationBar.Buttons.Add(appBarButton);

        //    // 使用 AppResources 中的本地化字符串创建新菜单项。
        //    ApplicationBarMenuItem appBarMenuItem = new ApplicationBarMenuItem(AppResources.AppBarMenuItemText);
        //    ApplicationBar.MenuItems.Add(appBarMenuItem);
        //}
    }
}
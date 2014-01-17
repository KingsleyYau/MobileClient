//#define DEBUG_AGENT
using System;
using System.Linq;
using System.Diagnostics;
using System.Windows;
using Microsoft.Phone.Scheduler;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using System.IO.IsolatedStorage;
using DrClientDll;
using CustomControl;
using System.Xml.Linq;
using System.Reflection;
using System.Windows.Media.Imaging;
using System.IO;

namespace BackgroundAgent
{

    public class ScheduledAgent : ScheduledTaskAgent
    {
        private static volatile bool _classInitialized;
        //private static uint nCount = 0;   fgx 2013-10-18 去掉没有用到的变量
        private DrClient _drClient = new DrClient();
      //  private TileBackImg customTile = new TileBackImg();
        private int schedulingCount;
    
        /// <remarks>
        /// ScheduledAgent 构造函数，初始化 UnhandledException 处理程序
        /// </remarks>
        static ScheduledAgent()
        {
            if (!_classInitialized)
            {
            //    nCount = 0;
               
                _classInitialized = true;
                
            }
            // 订阅托管的异常处理程序
            Deployment.Current.Dispatcher.BeginInvoke(delegate
            {
                Application.Current.UnhandledException += UnhandledException;
            });
        }

        /// 出现未处理的异常时执行的代码
        private static void UnhandledException(object sender, ApplicationUnhandledExceptionEventArgs e)
        {
            if (Debugger.IsAttached)
            {
                // 出现未处理的异常；强行进入调试器
                Debugger.Break();
            }
        }
        // 断线 更新磁贴状态
        public void UpdateTileState()
        {
            const string filename = "/Shared/ShellContent/NormalTileBack.jpg";
            Deployment.Current.Dispatcher.BeginInvoke(() =>
            {
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
                        BackTitle = normalImg.GetConnStatusTxt(),
                        BackContent = normalImg.GetTileContentTxt(),
                        BackBackgroundImage = new Uri("isostore:" + filename, UriKind.Absolute),
                    };
                    TileToFind.Update(tileData);
                }
                catch (Exception e)
                {
                    //MessageBox.Show(e.Message);
                }
           

            }
     });
        }

        // 已经登录成功的情况下更新磁贴状态
        public void UpdateTile(string usedTime, string usedFlux, string updateTime)
        {
            Deployment.Current.Dispatcher.BeginInvoke(() =>
                {
                    TileBackImg customTile = null;
                    try
                    {
                        customTile = new TileBackImg();
                    }
                    catch (System.Exception ex)
                    {
                        string strMsg = ex.Message;
                    }
                    customTile = new TileBackImg();
                    customTile.SetusedTime( usedTime);
                    customTile.SetusedFlux(usedFlux);
                    customTile.SetLastUpdateTime(updateTime);
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
                            BackContent = "",
                            BackBackgroundImage = new Uri("isostore:" + filename, UriKind.Absolute),
                        };

                        TileToFind.Update(tileData);
                    }
                    catch (Exception e)
                    {
                        //MessageBox.Show(e.Message);
                    }
                });
          
        }
        /// <summary>
        /// 运行计划任务的代理
        /// </summary>
        /// <param name="task">
        /// 调用的任务
        /// </param>
        /// <remarks>
        /// 调用定期或资源密集型任务时调用此方法
        /// </remarks>
        protected override void OnInvoke(ScheduledTask task)
        {
            string strGateWay = "";
          
            using (IsolatedStorageFile storage = IsolatedStorageFile.GetUserStoreForApplication())
            {
                XElement _xml;
                try
                {
                    IsolatedStorageFileStream location = new IsolatedStorageFileStream("settings", System.IO.FileMode.Open, storage);
                    // 转化为可读流
                    System.IO.StreamReader file = new System.IO.StreamReader(location);
                    // 解析流转化为XML
                    _xml = XElement.Parse(file.ReadToEnd());
                    strGateWay = _xml.Element("GatewayAddr").Value;
                    schedulingCount = Convert.ToInt32(_xml.Element("schedulingCount").Value);
                    file.Dispose();                               // 关闭可写流
                    location.Dispose();                       //关闭手机独立存储流
                }
                catch (System.Exception ex)
                {
                	
                }
       
     
            }
            //if (null==strGateWay || strGateWay.Length==0)
            //{
            //    return;
            //}
            
            _drClient.Init();
            // 读取上一次正常登录的网关地址
            string strGatewayAddr = strGateWay;


            if (schedulingCount % 2 == 0)
            {
                _drClient.SetGatewayAddress(strGatewayAddr);
                int nRet =_drClient.GetStatus();
                if ( (ClientHandleStatus)nRet == ClientHandleStatus.DrClient_Success)
                { 
                    // 成功获取到状态，更新磁贴
                    string flux = _drClient.GetFluxStatus();
                    string time = _drClient.GetTimeStatus();
                    string updatetime = DateTime.Now.ToString("HH:mm");
                    UpdateTile(time, flux, updatetime);

                }
                else if ( (ClientHandleStatus)nRet == ClientHandleStatus.DrClient_NetworkError ||
                    (ClientHandleStatus)nRet == ClientHandleStatus.DrClient_UndefineError)
                { 
                    // 网络已经断开， 更新磁贴
                    UpdateTileState();
                }    
            }
              schedulingCount++;
              using (IsolatedStorageFile storage = IsolatedStorageFile.GetUserStoreForApplication())
              {

                  if (storage.FileExists("settings"))
                  {
                      storage.DeleteFile("settings");
                  }

                  try
                  {
                      XElement _item = new XElement("Settings");
                      _item.Add(new XElement("GatewayAddr", strGateWay));
                      _item.Add(new XElement("schedulingCount", schedulingCount));
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



                 // XElement _xml;
                 // try
                 // {
                 //     IsolatedStorageFileStream location = new IsolatedStorageFileStream("settings", System.IO.FileMode.Open, FileAccess.ReadWrite,  storage);
                 //  //   StreamWriter filewrite = new StreamWriter(location);
                 //     StreamReader file = new StreamReader(location);
                 //     _xml = XElement.Parse(file.ReadToEnd() );
                 //     file.Dispose();
                 //     file.Close();
                 //     location.Dispose();
                 //     location.Close();

                 //     IsolatedStorageFileStream location2 = new IsolatedStorageFileStream("settings", System.IO.FileMode.Open, FileAccess.ReadWrite, storage);
                 //     StreamWriter filewrite = new StreamWriter(location2);
                 //     _xml.Element("schedulingCount").SetValue(schedulingCount);
                 //     _xml.Save(filewrite);
                 //     filewrite.Flush();
                 //     filewrite.Dispose();
                 //     filewrite.Close();
                      


                 ////     // 转化为可读流
                 ////     System.IO.StreamReader fileRead = new System.IO.StreamReader(location);
                 ////     // 解析流转化为XML
                 ////     _xml = XElement.Parse(fileRead.ReadToEnd());
                 ////     _xml.Element("schedulingCount").SetValue(schedulingCount);
                 ////     fileRead.Dispose();                               // 关闭可写流




                 ////     // 将修改后的XML写入文件
                 //// //    IsolatedStorageFileStream location2 = new IsolatedStorageFileStream("settings", System.IO.FileMode.Open, FileAccess.Write,storage);
                 ////     System.IO.StreamWriter fileWrite = new System.IO.StreamWriter(location);
                 ////     _xml.Save(fileWrite);
                 ////     fileWrite.Dispose();
                 ////     location.Dispose();
                 // //    location2.Dispose();


                      
                 // }
                 // catch (System.Exception ex)
                 // {
                 //     Debug.WriteLine(ex.Message);
                 // }


              }


            //TODO: 添加用于在后台执行任务的代码

            // If debugging is enabled, launch the agent again in one minute.
#if DEBUG_AGENT
            ScheduledActionService.LaunchForTest(task.Name, TimeSpan.FromSeconds(60));
#endif
            NotifyComplete();
        }
    }
}
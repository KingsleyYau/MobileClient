using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.IsolatedStorage;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace DrClient4Wp8
{

    class CurrentStatus
    {
        IsolatedStorageSettings settings;
        const string CurrentStatusSettingKeyName = "CurrentStatusSetting";
        const bool CurrentStatusSettingDefault = false;

        public CurrentStatus()
        {
            settings = IsolatedStorageSettings.ApplicationSettings;
        }

        public bool AddOrUpdateValue(string Key, Object value)
        {
            bool valueChanged = false;

            // 该键存在
            if (settings.Contains(Key))
            {
                // 键值有变化
                if (settings[Key] != value)
                {
                    // 更新键值
                    settings[Key] = value;
                    valueChanged = true;
                }
            }
            // 否则创建该键值
            else
            {
                settings.Add(Key, value);
                valueChanged = true;
            }
            return valueChanged;
        }

        public T GetValueOrDefault<T>(string Key, T defaultValue)
        {
            T value;
            // 该Key存在，返回对应值
            if (settings.Contains(Key))
            {
                value = (T)settings[Key];
            }
            // 否则返缺省值
            else
            {
                value = defaultValue;
            }
            return value;
        }

        // 保存设置
        public void Save()
        {
            settings.Save();
        }

        public bool CurrentStatusSetting
        {
            get
            {
                return GetValueOrDefault<bool>(CurrentStatusSettingKeyName, CurrentStatusSettingDefault);
            }
            set
            {
                if (AddOrUpdateValue(CurrentStatusSettingKeyName, value))
                {
                    Save();
                }
            }
        }

    }

    class AppSettings
    {
        // Our isolated storage settings
        IsolatedStorageSettings settings;

        // 要储存的设置

        const string UsernameSettingKeyName = "UsernameSetting";
        const string PasswordSettingKeyName = "PasswordSetting";
        const string SavePasswordSettingKeyName = "SavePasswordxSetting";
        const string AutoLoginSettingKeyName = "AutoLoginSetting";
        const string GatewayAddrSettingKeyName = "GatewayAddrSetting";

        const bool SavepasswordSettingDefault = false;
        const bool AutoLoginSettingDefault = false;
        const string UsernameSettingDefault = "";
        const string PasswordSettingDefault = "";
        const string GatewayAddrSettingDefault = "";

        // 构造函数
        public AppSettings() 
        {
            settings = IsolatedStorageSettings.ApplicationSettings;
        }

        public bool AddOrUpdateValue(string Key, Object value)
        {
            bool valueChanged = false;

            // 该键存在
            if (settings.Contains(Key))
            {
                // 键值有变化
                if (settings[Key] != value)
                {
                    // 更新键值
                    settings[Key] = value;
                    valueChanged = true;
                }
            }
            // 否则创建该键值
            else
            {
                settings.Add(Key, value);
                valueChanged = true;
            }
            return valueChanged;
        }

        public T GetValueOrDefault<T>(string Key, T defaultValue)
        {
            T value;
            // 该Key存在，返回对应值
            if (settings.Contains(Key))
            {
                value = (T)settings[Key];
            }
            // 否则返缺省值
            else
            {
                value = defaultValue;
            }
            return value;
        }

        // 保存设置
        public void Save() 
        {
            settings.Save();
        }

        // 属性设置

        public string UsernameSetting 
        {
            get
            {
                return GetValueOrDefault<string>(UsernameSettingKeyName, UsernameSettingDefault);
            }
            set
            {
                if (AddOrUpdateValue(UsernameSettingKeyName, value))
                {
                    Save();
                }
            }

        }
        public string PasswordSetting
        {
            get
            {
                return GetValueOrDefault<string>(PasswordSettingKeyName, PasswordSettingDefault);

            }
            set
            { 
                if(AddOrUpdateValue(PasswordSettingKeyName,value))
                {
                    Save();
                }
            }
        
        }
        public string GatewayAddrSetting
        {
            get
            {
                return GetValueOrDefault<string>(GatewayAddrSettingKeyName, GatewayAddrSettingDefault);
            }
            set
            {
                if (AddOrUpdateValue(GatewayAddrSettingKeyName, value))
                {
                    Save();
                }
            }
        
        }

        public bool SavePasswordSetting
        {
            get
            {
                return GetValueOrDefault<bool>(SavePasswordSettingKeyName, SavepasswordSettingDefault);
             }
            set
            {
                if (AddOrUpdateValue(SavePasswordSettingKeyName, value))
                {
                    Save();
                }
            }
        }


        public bool AutoLogindSetting
        {
            get
            {
                return GetValueOrDefault<bool>(AutoLoginSettingKeyName , AutoLoginSettingDefault);
            }
            set
            {
                if (AddOrUpdateValue(AutoLoginSettingKeyName, value))
                {
                    Save();
                }
            }
        }

    }
}

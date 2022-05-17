# iot_gateway
The ESP8266 gateway which can control the relay by sonar output and mqtt commands

# 基本需求

## 通过mqtt发布和订阅配置/状态

* 参考如下发布和订阅的主题

```python
# 所有外发的状态字典变量
main_status = {
'localtime_str':            "",
'relay_timing_on_enable':   False,
'relay_timing_on_time':     "",
'relay_timing_on_enable':   False,
'relay_timing_on_time':     "",
'relay_status':             False,
'wdt_enabled':              False,
'distance':0,
'wifi_status':'',
'internet_status':'',
'current_ssid':''
}

# 所有外发/订阅的配置字典变量
main_config = {
'wdt_config':None,
'relay_config':None
}
```

## 读取超声波测距，测出的距离

* 实现超声波测距
* 将距离信息显示到网页
* 将距离信息通过mqtt上报

## 本地网页

### 主页面

* 状态信息

```
状态名称	当前状态
系统时间:	{{ localtime_str }}
Wifi状态:	{{ wifi_status }}
Internet状态:	{{ internet_status }}
SSID:	{{ current_ssid }}
看门狗:	{{ wdt_status }}
原始距离(cm):	{{ original_distance }}
```

* wifi配置页面按钮

* 设置和状态控制页面按钮

### wifi配置页面

* 返回主页面按钮

* 刷新wifi信息按钮

* wifi信息选择及密码输入

* 提交按钮

### 设置和状态控制页面

* 返回主页面按钮

* 刷新状态按钮

* 继电器设置勾选框

* 提交按钮

# 程序烧写

Arduino自带的烧写工具可能无法满足实际应用的烧写需求。所以可以先编译生成bin文件，再用ESP官方烧写工具写入ESP8266



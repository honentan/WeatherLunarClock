# WeatherLunarClock
Clock with 0.96oled - Digital Clock with weather forecast and China lunar calendar

ESP8266+0.96寸OLED，显示数字时钟、实时天气、明后天气、农历、节气、工作日，以及实测温湿度。

修改key_secret.h中的wifi信息和心知天气密钥，农历api密钥。

完成效果看这里：
https://blog.csdn.net/honentan/article/details/131200537

更新：
用wifiManager配网，启动后Wifi连接AutoConnect，打开192.168.4.1进行wifi设置。
23点至次日6点关闭OLED，可通过触摸TPP233打开屏幕20秒。
温湿度检测模块SHT40和触摸开关TTP233可通过编译选项配置。

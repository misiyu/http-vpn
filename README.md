# http-vpn

准备工作：
1、安装NFD
2、配置：
	$ sudo vim /usr/local/etc/ndn/nfd.conf 
	查找到以下行:
	cs_unsolicited_policy XXX   (XXX表示任意值)
	改为:
	cs_unsolicited_policy admit-local
3、重启NFD
	$ nfd-stop
	$ nfd-start

运行：
1、打开client：
	（1）进入client文件夹 
	（2）$ make
	（3）$ ./client 

2、打开server
	（1）进入server文件夹 
	（2）$ make 
	（3）$ ./server 
测试 ：

1、命令行：
	curl -v --socks5 127.0.0.1:8888  https://www.baidu.com
	curl -v --socks5 127.0.0.1:8888  https://dlie.sogoucdn.com/se/sogou_explorer_8.6_0903.exe -o test.exe
	http://mirrors.huaweicloud.com/ubuntukylin/ubuntukylin-19.04-enhanced-amd64.iso

2、浏览器：
	设置浏览器socks代理端口为8888


进展 
1、2019.09.10 本地内部初步调通，但速率有点慢
2、2019.09.11 Nack无法停止的情况

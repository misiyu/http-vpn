# http-vpn

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

2、浏览器：
	设置浏览器socks代理端口为8888


1、2019.09.10 本地内部初步调通，但速率有点慢

## 程序发布，打包依赖的dll文件

1.在windows系统开始主菜单里进入对应的依赖的cmd命令行程序，比如MinGW

2.新建发布的目录和需要打包的exe文件到空文件中，并cd 到该目录

3.执行
```    
windeployqt app.exe
```


## 通信协议格式

文件服务器返回头文件
```

QList(std::pair("Server","SimpleHTTP/0.6 Python/3.11.4"), std::pair("Date","Tue, 27 Feb 2024 11:26:40 GMT"), std::pair("Content-type","text/html; charset=utf-8"), std::pair("Content-Length","263"))

QList(std::pair("Server","SimpleHTTP/0.6 Python/3.11.4"), std::pair("Date","Tue, 27 Feb 2024 11:28:12 GMT"), std::pair("Content-type","text/plain"), std::pair("Content-Length","4890050"), std::pair("Last-Modified","Mon, 15 Jan 2024 08:35:43 GMT"))

```

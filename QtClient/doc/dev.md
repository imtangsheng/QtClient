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


## 样式表

### QSlider
groove是指滑动条的轨道部分，而horizontal是指水平方向的滑动条。

在样式表中，你可以使用QSlider::groove选择器来选择滑动条的轨道部分，并设置其样式。以下是一些常用的样式属性和选择器：
```
QSlider::groove：选择滑动条的轨道部分。
QSlider::handle：选择滑动条的滑块部分。
QSlider::sub-page：选择滑动条的滑过部分（滑块之前的部分）。
QSlider::add-page：选择滑动条的未滑过部分（滑块之后的部分）。
你可以使用这些选择器来设置滑动条的不同部分的样式，例如背景颜色、边框样式、大小等。

以下是一些常用的样式属性，你可以在样式表中使用它们来设置滑动条的外观：

background-color：设置背景颜色。
border：设置边框样式。
width：设置宽度。
height：设置高度。


QSlider::groove:horizontal {
    border: 1px solid #999999;
    height: 8px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);
    margin: 2px 0;
}

QSlider::groove:vertical {
    background: red;
    position: absolute; /* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */
    left: 4px; right: 4px;
}
```

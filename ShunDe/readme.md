# Developing 开发文档

## 更新日志
    rtsp流需要自己设置显示视频流方式

+ 日期: 2024/7/6
    - bug修改
    - 1.处理tcp客户端连接的设备添加要在主线程，使用信号与槽机制
    - 2.处理tcp断开的异常
    - 暂时未处理：
    - 1.tcp客户端要求在一个线程，不可异步线程的警告，无影响程序
    - 2.在等待接收信息过程中客户端快速断开程序会奔溃，可能需要自己实现一个线程池类管理
    
+ 日期: 2024/6/27
    - 1.修改增加设置录像，抓拍全局地址
    - 2.增加区域入侵录像的事件的支持
    - 3.增加实现自动登录的功能，**使用同步登录会阻塞主线程**
    
+ 日期: 2024/6/26
    - 1.使用Qt 中异步执行的加载，在事件队列中异步执行。不可在这里添加信号连接，定时器，会立即触发，定时器失效。
    - 2.增加修改海康的报警事件显示，还有增加录像功能

+ 日期: 2024/6/14   
    - 增加报警显示统计数据的柱状图标签显示方法
    
+ 日期: 2024/6/13
    - 1.列宽度调整模式,它的作用是根据表格内容自动调整列宽,使所有列宽度相等。
    - 2.使用图片缓存计算来优化在加载渲染界面卡顿的问题

+ 日期: 2024/6/8
    - bug排查
        - 1.数据库默认连接，在使用要指定数据库以免报错db未打开
        - 2.NetworkReply 是一个抽象基类，不能直接创建。使用指针类型，局部使用。
+ 日期: 2024/6/5
    - 1.海康的xlsxio库,仅支持写入数据（不支持公式、布局、图形、图表等）,不支持多个工作表（每个文件只有一个工作表    所以不使用。在使用过程中，需要把dll文件放到同exe文件目录
    - 2.支持导出文件，csv使用"|" 间隔，python脚本把csv文件转换为excel文件
    - 3.巡检任务快捷拍照设置支持
    - 4.动态dll使用需要拷贝dll到exe一个目录，静态不用
    - 5.添加录像模块的脚本

+ 日期: 2024/5/29
    1.巡检任务调试完成
    
+ 日期: 2024/5/24
    1.增加地图显示模块
    2.增加巡检任务信息界面

+ 日期: 2024/5/20
    - 添加数据库筛选项的功能,按时间，按值筛选
    
+ 日期: 2024/5/17
        - 巡检时间设置支持天数，星期来设置
        
+ 日期: 2024/5/11
    - 新增巡检线程函数功能模块

+ 日期: 2024/5/9
    - 1.巡检的配置设置
    
+ 日期: 2024/4/30
    - 1.巡检和设置ui界面设计
    
+ 日期: 2024/4/29
    - 1.增加数据报表设置ui，参数设置ui
    - 2.修改ui变量为全局变量
+ 日期: 2024/4/26
    - 1.添加配置中心的界面
    - 2.添加数据库
    - 3.数据报表显示，事件中心显示

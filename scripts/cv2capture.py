"""## 说明
- 本脚本用于从RTSP视频流中抓取图片、播放视频、录制视频
- 本脚本使用opencv-python库
- 本脚本使用python3.7.3开发
- 本脚本使用logging模块记录日志
- 本脚本使用threading模块开启线程
- date: 2023-07-20
- author: tang

- version: 0.0.4
1.支持单通道，多摄像头，配置文件和程序名同名

- version: 0.0.1
1.取消函数内线程，直接使用函数
2.增加多线程抓取视频
- version: 0.0.2
1.增加日志文件管理
2.增加读取配置文件

- version: 0.0.3
1.区别分路劲文件和路径文件夹
2.瀑捕获错误：Could not find ref with POC 47
这个错误表示在解码 HEVC(H.265) 视频时,找不到指定 POC (Picture Order Count) 的参考帧。
POC 是 HEVC 中用来表示帧顺序的一个计数器,解码时需要根据 POC 值找到对应的参考帧来解码 inter 帧。
通常会在网络不好的情况下出现这个错误,因为网络不好会导致丢包,丢包会导致解码失败。
3.print函数重定向到日志文件

"""
import cv2
import datetime
import os
import time
import threading


class CV2VideoHandler:
    running = True
    reconnect = False

    def capture_is_opened(self, url: str) -> bool:
        capture = cv2.VideoCapture(url)
        ret = capture.isOpened()
        if ret:
            if self.reconnect:
                print('Reconnected to video stream %s', url)
                self.reconnect = False
        capture.release()
        return ret

    def save_image(self, url: str, image_path: str = 'video', image_type='jpg') -> bool:

        capture = cv2.VideoCapture(url)
        ret, frame = capture.read()
        if ret:
            filename = self.get_filename(image_path, image_type)
            cv2.imwrite(filename, frame)
            print('Saved image to %s', filename)
        else:
            print('Error reading video stream from %s', url)
        capture.release()

    def play_video(self, url: str, window_name='frame'):

        capture = cv2.VideoCapture(url)

        while True:
            ret, frame = capture.read()

            if ret:
                cv2.imshow(window_name, frame)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
            else:
                break

        capture.release()
        cv2.destroyAllWindows()

    def record_video(self, url, video_dir, video_type='avi',
                     max_size=1*1024*1024*1024, wait_reconnect=10):

        filename = self.get_filename(video_dir,  video_type)
        cap = cv2.VideoCapture(url)
        if not cap.isOpened():
            print('Error opening video stream %s', url)
            cap.release()
            return
        
        flag_reconnect = True
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        fps = cap.get(cv2.CAP_PROP_FPS)

        vout = cv2.VideoWriter(filename, cv2.VideoWriter_fourcc(
            *'XVID'), fps, (width, height))
        self.running = True
        print('Start recording video to %s', filename)
        while self.running:
            ret, frame = cap.read()
            if ret:
                try:
                    vout.write(frame)
                    size = os.path.getsize(filename)
                    if size >= max_size:
                        print('Reached max file size, files: %s' , filename)
                        vout.release()
                        # self.rotate_file(filename, video_dir, video_type)
                        filename = self.get_filename(video_dir, video_type)
                        vout = cv2.VideoWriter(filename, cv2.VideoWriter_fourcc(*'XVID'), fps, (width, height))
                except Exception as e:
                    print(e)
                    # self.running = False
            else:
                while self.running:
                    if flag_reconnect:
                        print('Error reading video stream ,file:%s. reconnecting in %d secs...',filename, wait_reconnect)
                        flag_reconnect = False
                    time.sleep(wait_reconnect)
                    # 重新连接
                    # cap = cv2.VideoCapture(url)
                    cap.open(url)
                    if cap.isOpened():
                        print('Reconnected to video stream %s', filename)
                        flag_reconnect = True
                        # self.rotate_file(filename, video_dir, video_type)
                        # filename = self.get_filename(video_dir, video_type)
                        # vout = cv2.VideoWriter(filename, cv2.VideoWriter_fourcc(*'XVID'), fps, (width, height))
                        break

        cap.release()
        vout.release()
        print('Stopped recording: %s', video_dir)

    # def record_video_thread(self, url, video_dir, video_type='avi',
    #                  max_size=1*1024*1024*1024, wait_reconnect=10):
    #     thread = threading.Thread(target=self.record_video, args=(url,video_dir, video_type, max_size, wait_reconnect))
    #     thread.start()
    #     thread.join()

    def get_filename(self, path: str, file_type: str) -> str:
        now = datetime.datetime.now()
        name = now.strftime('%Y%m%d_%H%M%S.'+file_type)
        path = os.path.join(path, now.strftime('%Y%m'), now.strftime('%Y%m%d'))
        filename = os.path.join(path, name)
        if not os.path.exists(path):
            os.makedirs(path)
        return filename

    def rotate_file(self,filename, video_dir, video_type):
        now = datetime.datetime.now()
        new_name = now.strftime('%Y%m%d_%H%M%S.'+video_type)
        os.rename(filename, os.path.join(video_dir, new_name))

    def close(self):
        self.running = False

    def __del__(self):
        self.close()


if __name__ == '__main__':
    # RTSP视频流URL
    # // 说明：
    # // username：用户名，例如admin
    # // password：密码，例如12345
    # // ip：设备的ip地址，例如192.0.0.64
    # // port：端口号默认554，若为默认可以不写
    # // codec：有h264、MPEG-4、mpeg4这几种
    # // channel：通道号，起始为ch1,ch2
    # // subtype：码流类型，主码流为main，子码流为sub

    # 2012年之前的设备
    # rtsp://[username]:[passwd]@[ip]:[port]/[codec]/ch[channel]/[subtype]/av_stream
    # 新设备
    # rtsp://[username]:[passwd]@[ip]:[port]/Streaming/Channels/[channel](?parm1=value1&parm2=value2)
    # channel: 通道号，从1开始,101表示通道1的主码流，102表示通道1的子码流，以此类推
    ip = "192.168.1.36"
    port = "554"
    username = "admin"
    password = "dacang80"
    codec = "h264"
    channel = "1"
    subtype = "main"

    path_video = "video"
    path_infrared = "infrared"
    filename_log = "video.log"

    url = "rtsp://%s:%s@%s:%s/Streaming/Channels/" % (
        username, password, ip, port)
    url_old = "rtsp://%s:%s@%s:%s/%s/ch%s/%s/av_stream" % (
        username, password, ip, port, codec, channel, subtype)

    """## 读取配置文件
    """
    import json

    # 获取当前文件所在的目录
    current_dir = os.path.dirname(__file__)
    config_path = os.path.join(current_dir, 'cv2capture.json')

#    current_path = os.getcwd()
#    config_path = os.path.join(current_path, "config/config.json")
    # 读取json配置
    with open(config_path, mode='r', encoding='utf-8') as f:
        config = json.load(f)

    videolog_file = config['log']['videolog_file']
    if not os.path.isfile(videolog_file):
        open(videolog_file, 'w').close()

    capture_config = {
        "ip": "192.168.1.36",
        "port": 8000,
        "port_rtsp": 554,
        "user": "admin",
        "password": "dacang80",
        "video_dir": "D:/video/",
        "channels": [
            {
                "channel": "102",
                "video_dir": "video",
                "video_type": "avi",
                "max_size": 1*1024*1024*1024,
                "wait_reconnect": 5
            },
            {
                "channel": "202",
                "video_dir": "infrared",
                "video_type": "avi",
                "max_size": 1*1024*1024*1024,
                "wait_reconnect": 5

            }]
    }

    """## 多线程抓取视频
    """
    global funcs
    funcs = []
    for device in config['hikvision']:
        for key in device.keys():
            if key in capture_config.keys():
                # capture_config[key] = device[key]
                capture_config.update({key: device[key]})
        print('start capture:'+capture_config['ip'])
        video_handler = CV2VideoHandler()
        funcs.append(video_handler)
        url = "rtsp://%s:%s@%s:%s/Streaming/Channels/" % (
            capture_config['user'], capture_config['password'], capture_config['ip'], capture_config['port_rtsp'])
        # url_old = "rtsp://%s:%s@%s:%s/%s/ch%s/%s/av_stream" % (username, password, ip, port, codec, channel, subtype)
        for channel in capture_config['channels']:
            # print(channel)
            thread = threading.Thread(target=video_handler.record_video, args=(
                url+channel['channel'], capture_config['video_dir']+capture_config['ip']+channel['video_dir'], channel['video_type'], channel['max_size'], channel['wait_reconnect']))
            thread.start()
            # 等待线程终止
            # thread.join()

    """## 日志文件管理
    Formatter主要有以下内置的格式化字符串:
        - %(name)s - 打印日志器名称
        - %(levelno)s - 打印日志级别的数值 
        - %(levelname)s - 打印日志级别名称
        - %(pathname)s - 打印日志调用路径
        - %(filename)s - 打印路径中的文件名
        - %(module)s - 打印模块名称
        - %(funcName)s - 打印日志调用函数名称
        - %(lineno)d - 打印日志调用行号
        - %(created)f - 打印日志创建时间,默认精度毫秒
        - %(asctime)s - 打印日志创建时间,可自定义格式
        - %(msecs)d - 打印日志创建时间,毫秒部分
        - %(relativeCreated)d - 打印日志创建时间,相对于logging模块加载时间
        - %(thread)d - 打印线程ID
        - %(threadName)s - 打印线程名称
        - %(process)d - 打印进程ID
        - %(message)s - 打印日志消息
    """
    import logging
    from logging.handlers import TimedRotatingFileHandler

    logger = logging.getLogger('VideoLogger')
    logger.setLevel(logging.DEBUG)

    LOGGING_MSG_FORMAT = '%(asctime)s - %(module)s - %(funcName)s - %(lineno)d - %(message)s'
    LOGGING_DATE_FORMAT = '%m-%d %H:%M:%S'
    formatter = logging.Formatter(LOGGING_MSG_FORMAT, LOGGING_DATE_FORMAT)

    # 设置utf-8编码
    # handler = TimedRotatingFileHandler('logs/video.log', encoding='utf-8', when='M', interval=1)
    handler = TimedRotatingFileHandler(
        videolog_file, encoding='utf-8', when='midnight', interval=1)
    # 设置日志文件名为 video
    # handler.baseFilename = 'video'
    # # 自行拼接日志文件名
    handler.suffix = "%Y-%m-%d.log"

    # handler.namer = lambda name: name + ".log"
    # handler.namer = lambda name: name.replace(".log", "") + ".log"
    # CRITICAL = 50
    # FATAL = CRITICAL
    # ERROR = 40
    # WARNING = 30
    # WARN = WARNING
    # INFO = 20
    # DEBUG = 10
    # NOTSET = 0
    LOG_LEVEL = {'info':20,'debug':logging.DEBUG,'warning':30,'error':40}
    handler.setLevel(LOG_LEVEL.get(config['log']['level'],logging.DEBUG))

    handler.setFormatter(formatter)
    logger.addHandler(handler)

    handler.doRollover()
    print = logger.info

    # 程序启动会比较慢，等待一段时间才可能关闭，所以这里等待10秒
    # time.sleep(10)

    while True:
        time.sleep(10)
        key = input('Press q to quit:')
        if key == 'q':
            break
        print('Press q to quit:')
    print('Closing...')
    print('Done')
    logger.info('Done')

    # 关闭程序
    for func in funcs:
        func.close()
    logging.shutdown()

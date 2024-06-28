"""#使用朗驰IR接口的热成像报警http服务器实现
##使用安装环境
pip install fastapi uvicorn
pip install requests
##

Project: FastAPI Application
Description: A simple FastAPI application for managing posts.
Author: Tang

Version: 0.0.1
Updated: June 25, 2024


"""

from fastapi import APIRouter,Request,Body
from pydantic import BaseModel
import requests
s = requests.Session()
s.timeout = 5

import base64
import datetime
import json
#获取抓图方法使用http请求抓图
User = "21218CCA77804D2BA1922C33E0151105"
Key = "21218CCA77804D2BA1922C33E0151105"
path_snap = "E:DCRobot/snap/"
ip_camera = "192.168.1.19"
url_token = "http://"+ip_camera+"/api/v1/token"
url_snapParam =  "http://"+ip_camera+"/api/v1/device/sanpParam"
url_snap = "http://"+ip_camera+"/api/v1/channel/snap/0"
def get_token(url:str) -> dict:
    json_token = {"user":User,"key":Key}
    req = requests.post(url,json=json_token)
    if req.status_code == 200:
        req_json = req.json()
        if(req_json['code'] == 200):
            return req_json['data']
    return None

def get_snap(token:dict,filename:str) -> str:
    headers = {"accessToken":token["accessToken"]}
    req_snap = requests.get(url_snap,headers=headers)
    if req_snap.status_code==200:       
        try:
            img = req_snap.json()
            img_data = base64.b64decode(img['data']['snapData'])
            with open(path_snap + filename,'wb') as f:
                f.write(img_data)
            return path_snap + filename
        except json.JSONDecodeError:
            print("Error:Unable to decode Json response")
    return None

def get_snap_path(time_str:str) ->str:
    img_path = None
    session = requests.Session()
    token = get_token(url_token)
    if token:
        time_obj = datetime.datetime.strptime(time_str,"%Y-%m-%d %H:%M:%S")
        filename = time_obj.strftime("%Y_%m_%d_%H%M%S") + ".jpg"
        img_path = get_snap(token,filename)
    session.close()
    return img_path
#获取抓图方法使用http请求抓图

class EventCenter(BaseModel):
    time: str
    source: str
    type: str
    level: int
    details: str
    status: str

# 第三方服务器的URL
third_party_server_url = "http://127.0.0.1:31024/event"
def send_alarm_to_third_party(alarm_data:EventCenter):
    try:
        response = s.post(third_party_server_url, json=alarm_data.dict())
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Error sending alarm to third party server: {e}")

# 创建Router实例
router = APIRouter(prefix="/api/v1", tags=["朗驰 Alarm Server"])

# 使用朗驰IR接口热成像报警
@router.post("/alarmserver/status")
def IR_alarmserver(request:Request,data: dict = Body(...)):
    """## 使用朗驰IR接口热成像报警
    - Parameters:
        - 无
    ## 返回值:
    - response:EventCenter
        - time: str(yyyy-MM-dd hh:mm:ss)
        - source: str
        - type: str
        - level: int
        - details: str
        - status: str
    """
    # 打印整个请求体
    print("请求来源的IP地址：", request.client.host)
    print("请求体：",data)
    if "utcTime" in data: 
        print("utcTime",data["utcTime"])
    #温度报警
    if "globTemp" in data:
        print("globTemp:", data["globTemp"])
        if(data["globTemp"]["alarm"]):
            print("报警值:", data["globTemp"]["value"]) #报警值
            print("报警type:",data['globTemp']['threshold']['type']) #'high'
            print("报警最高温度阈值:",data['globTemp']['threshold']['highTemp']) #报警最高温度阈值
            print("报警最低温度阈值:",data['globTemp']['threshold']['lowTemp'])
            alarm_data = EventCenter(
                time=data["utcTime"],
                source=request.client.host,
                type="温度报警",
                level=2,
                details=f"温度报警: {data['globTemp']['value']}°C 图片：" + get_snap_path(data["utcTime"]),
                status=""
            )
            print("alarm_data",alarm_data)
            send_alarm_to_third_party(alarm_data)

    #火点报警
    if "firePoint" in data:
        print("firePoint:", data["firePoint"])
        if(data["firePoint"]["alarm"]):
            print("报警值:", data["firePoint"]["value"]) #报警值
            print("报警最高温度阈值:",data['firePoint']['threshold']) #报警最高温度阈值
            alarm_data = EventCenter(
                time=data["utcTime"],
                source=request.client.host,
                type="火点报警",
                level=2,
                details=f"火点报警: {data["firePoint"]["value"]}°C 图片："  + get_snap_path(data["utcTime"]),
                status=""
            )
            print("alarm_data",alarm_data)
            send_alarm_to_third_party(alarm_data)


if __name__ == "__main__":
    from fastapi import FastAPI
    import uvicorn
    app = FastAPI()
    # 将Router添加到主应用
    app.include_router(router)
    uvicorn.run(app, host="0.0.0.0", port=8080)

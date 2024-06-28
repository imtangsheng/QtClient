#include "httpserver.h"

#include <QHttpServerRequest>
#include <QJsonDocument>
#include <QSharedMemory>
#include <QUrl>
#include "sqlite.h"

//懒汉式，在第一次使用时才创建单例实例，但需要额外的同步机制来保证线程安全
//饿汉式，在类加载时就创建单例实例，线程安全
HttpServer* HttpServer::m_instance = nullptr;
HttpServer* gHttpServer;

HttpServer::HttpServer(QObject *parent)
    : QObject{parent}
{
    qDebug() << "HttpServer::HttpServer";
}

HttpServer::~HttpServer()
{
    qDebug() << "~HttpServer()";
}

HttpServer *HttpServer::instance(QObject *parent, const int &port)
{
    if(m_instance == nullptr){
        static QSharedMemory httpserver_instance("httpserver_instance");
        if (httpserver_instance.attach(QSharedMemory::ReadOnly)) {
            memcpy(&m_instance, httpserver_instance.data(), sizeof(HttpServer*));
        }else{
            if (httpserver_instance.create(sizeof(HttpServer*))) {
                std::once_flag flag;
                std::call_once(flag,[=]{
                    m_instance = new HttpServer(parent);
                    m_instance->init(port);
                    httpserver_instance.lock();
                    memcpy(httpserver_instance.data(), &m_instance, sizeof(HttpServer*));
                    httpserver_instance.unlock();
                    //httpserver_instance.detach();//如果这是附加到共享内存段的最后一个进程，则系统将释放共享内存段，即内容被销毁。
                });
            }
        }
    }
    qDebug() << "HttpServer::instance()"<<m_instance;
    return m_instance;
}

void HttpServer::shutdownHandler()
{
    qDebug() << "HttpServer:shutdownHandler()";
    if(m_instance != nullptr ){
        //delete instance;//静态变量,它是一个裸指针,没有任何智能指针管理它的生命周期。可能指向一个已经被其他地方删除的对象,可能会导致内存泄漏或其他严重的内存管理问题。这是因为 instance 指针没有跟踪对象的所有权,无法确保对象被正确地释放。
        delete HttpServer::instance(); // 析构函数会被调用,从而释放相关的资源
        //m_instance->deleteLater();//将 HttpServer 对象的删除操作添加到事件队列中,并返回立即,一个事件循环中,Qt 会从事件队列中取出这个删除操作,并执行 delete m_instance; 来释放 HttpServer 对象。需要手动调用
        m_instance = nullptr;
    }
}

void HttpServer::init(const int &port)
{
    const auto isPort = server.listen(QHostAddress::Any,31024);
    if(!isPort){
        qDebug()<< "Server failed to listen on a port."<<isPort;
        return;
    }
    qDebug()<< "Server listening on port."<<isPort<<port;
    route_test();
}

void HttpServer::route_test()
{
    qDebug()<< "HttpServer::init_route_laughing()";
    //get
    server.route("/",[](){
        qDebug()<< "server.route get:"<<u"/test/";
        QJsonObject response;
        response["path"] = "/test";
        return response;
    });

    server.route("/key", [](const QHttpServerRequest& request){
        QString key;
        QUrlQuery query(request.url().query());
        key = query.queryItemValue("key").toUtf8();
        qDebug()<<query.toString();
        qDebug()<<query.queryItemValue("key");
        return key;

    });
    server.route("/user/", [] (const qint32 id) {
        return QString("User %1").arg(id);
    });
    server.route("/user/<arg>/detail", [] (const qint32 id) {
        return QString("User %1 detail").arg(id);
    });
    server.route("/post",QHttpServerRequest::Method::Post,[](const QHttpServerRequest& request){
        qDebug()<< "server.route post:"<<request.body()<<request.query().queryItems();
        QJsonObject response;
        response["path"] = "/post";
        QJsonDocument jsonDoc = QJsonDocument::fromJson(request.body());
        if(!jsonDoc.isNull()){
            response["body"] = jsonDoc.object();
        }

        return response;
    });
}

void HttpServer::route_add_sql()
{
    server.route("/event",QHttpServerRequest::Method::Post,[=](const QHttpServerRequest& request){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(request.body());
        if(jsonDoc.isNull()){
            struJsonResponse.status = false;
            struJsonResponse.message = "无法解析的json格式";
            return struJsonResponse.toJson();

        }
        QJsonObject eventObj = jsonDoc.object();
        EventCenterData event;
        event.time = QDateTime::fromString(eventObj["time"].toString(), "yyyy-MM-dd hh:mm:ss");
        event.source = eventObj["source"].toString();
        event.type = eventObj["type"].toString();
        event.level = EventLevel(eventObj["level"].toInt());
        event.details = eventObj["details"].toString();
        event.status = eventObj["status"].toString();
        gSql->add_EventCenter(event);

        struJsonResponse.message = "OK";
        return struJsonResponse.toJson();
    });
}

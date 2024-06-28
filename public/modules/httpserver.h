#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QHttpServer>
#include <QJsonObject>

// 在一个头文件中定义 JsonResponse 结构体
struct Json_Response {
    bool status;
    QString message;
    QJsonObject data;
    QString other;

    QJsonObject toJson() const {
        return QJsonObject{
            {"status", status},
            {"message", message},
            {"data", data},
            {"other", other}
        };
    }
};

class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    static HttpServer *instance(QObject *parent = nullptr,const int& port = 8080);
    static void shutdownHandler();

    void init(const int& port = 8080);
    void route_test();
    void route_add_sql();

signals:

private:
    QHttpServer server;
    Json_Response struJsonResponse;
    static HttpServer* m_instance;
};
extern HttpServer* gHttpServer;
#endif // HTTPSERVER_H

#ifndef FILESUTIL_H
#define FILESUTIL_H

#include <QFileInfo>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>

class FilesUtil : public QObject
{
    Q_OBJECT
public:
    explicit FilesUtil(QObject *parent = nullptr);
    ~FilesUtil();

    void setRootUrl(const QString &url);

    void startRequest(const QUrl &requestedUrl);

    void cancelRequest();
//    QUrl::fromLocalFile
    void readAllFilesFromLocalPath(const QString &directory);

//    不需要额外处理new/delete内存 数据量小,直接定义
    QStringList filesListLocal;
    QStringList filesListNetwork;

    QStringList getFilesListLocalPath(const QString &path);
    QStringList getFilesListNetworkPath(const QString &url);

    void parseFilesListNetworkPath(const QString &url);

public slots:
//    void downloadFileFinished();

private slots:
    void downloadFile(const QString &url);

private:

    void initNetwork();
    // 创建一个 QNetworkAccessManager
    QNetworkAccessManager *m_networkAccessManager;
    // 发送 GET 请求
    QUrl url;
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> networkReply;
    std::unique_ptr<QFile> fileNetwork; //被赋值或移动后,原有对象的智能指针管理将自动解除 自动管理QFile对象的内存
    bool httpRequestAborted = false;
    
    void NetworkReplyFinished();
    void NetworkReplyReadyRead();
    void NetworkReplyDownloadProgress(int bytesReceived, int bytesTotal);


};

#endif // FILESUTIL_H

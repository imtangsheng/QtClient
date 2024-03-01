#ifndef FILESUTIL_H
#define FILESUTIL_H

#include <QWidget>
#include <QFile>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QHBoxLayout>

namespace Ui {
class FilesUtil;
}

class FilesUtil : public QWidget
{
    Q_OBJECT
public:
    explicit FilesUtil(QWidget *parent = nullptr);
    ~FilesUtil();

    void init();
    QWidget* getLayoutDownloadFile();

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
    bool downloadFileFromNetworkLink(const QUrl &link);
    void startDownloadFileFromLink(const QString &fileLink,const QString &filename);

public slots:
//    void downloadFileFinished();

private slots:
    void downloadFile(const QString &url);

    void on_checkBox_downloadFinished_stateChanged(int arg1);

private:
    Ui::FilesUtil *ui;

    void initNetwork();
    // 创建一个 QNetworkAccessManager
    QNetworkAccessManager *m_networkAccessManager;
    // 发送 GET 请求
    QUrl url;
    std::unique_ptr<QFile> file;

    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> networkReply;
    std::unique_ptr<QFile> fileNetwork; //被赋值或移动后,原有对象的智能指针管理将自动解除 自动管理QFile对象的内存
    bool httpRequestAborted = false;
    
    void NetworkReplyFinished();
    void NetworkReplyReadyRead();
    void NetworkReplyDownloadProgress(int bytesReceived, int bytesTotal);


};

#endif // FILESUTIL_H

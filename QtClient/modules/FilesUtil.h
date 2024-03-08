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

namespace Ui
{
    class FilesUtil;
}

class FilesUtil : public QWidget
{
    Q_OBJECT
public:
    explicit FilesUtil(QWidget *parent = nullptr);
    ~FilesUtil();

    void init();
    QWidget *getLayoutDownloadFile();

    void startRequest(const QUrl &requestedUrl);
    //    QUrl::fromLocalFile
    void readAllFilesFromLocalPath(const QString &directory);

    QString currentFilePath;
    QString currentFileLink;
    //    不需要额外处理new/delete内存 数据量小,直接定义
    QStringList filesListLocal;
    QStringList filesListNetwork;

    QStringList getFilesListLocalPath(const QString &path);
    QStringList getFilesListNetworkPath(const QString &url);

    void parseFilesListNetworkPath(const QString &url);
    bool downloadFileFromNetworkLink(const QUrl &link);
    bool startDownloadFileFromLink(const QString &fileLink, const QString &filename);

    enum class DownloadState
    {
        Download_Init, //
        Download_Start,
        Download_Downloading,
        Download_Finished,
        Download_Cancel,
        Download_Reset,
        Download_Error,
    };
    void updateDownloadState(DownloadState state);

public slots:
    //    void downloadFileFinished();

private slots:

    void on_checkBox_downloadFinished_stateChanged(int arg1);

    void on_pushButton_cancelDownload_clicked();

    void on_pushButton_openFromLocalFile_clicked();

    void on_pushButton_resetDownload_clicked();

private:
    Ui::FilesUtil *ui;
    // 创建一个 QNetworkAccessManager
    QNetworkAccessManager networkAccessManager;
    // 发送 GET 请求
    std::unique_ptr<QFile> file;

    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> networkReply;
    std::unique_ptr<QFile> fileNetwork; // 被赋值或移动后,原有对象的智能指针管理将自动解除 自动管理QFile对象的内存
    bool hasHttpRequest = false;        // http 终止标志

    void NetworkReplyFinished();
    void NetworkReplyReadyRead();
    void NetworkReplyDownloadProgress(int bytesReceived, int bytesTotal);
};

#endif // FILESUTIL_H

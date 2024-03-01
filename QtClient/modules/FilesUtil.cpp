#include "FilesUtil.h"
#include "ui_FilesUtil.h"

#include <QDir>
#include <QEventLoop>
#include <QFileInfoList>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QRegularExpression REGEX_LINKS("<a[^>]+href=['\"]([^'\"]+)['\"][^>]*>");

// 定义一个正则表达式模式，用于匹配网络下载链接
QRegularExpression REGEX_IS_LINK("(http|https|ftp)://.+\\.([^/$.]+)$");

FilesUtil::FilesUtil(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::FilesUtil)
{
    ui->setupUi(this);
    init();
    initNetwork();
}

FilesUtil::~FilesUtil()
{
    qDebug()<<"~FilesUtil()";
    delete ui;
}

void FilesUtil::init()
{

}

QWidget *FilesUtil::getLayoutDownloadFile()
{
//    QHBoxLayout *layout = ui->horizontalLayout_DownloadFiles;
    return ui->widget_downloadFile;
}

void FilesUtil::startDownloadFileFromLink(const QString &fileLink,const QString &filename)
{
    qDebug()<<"startDownloadFileFromLink(const QString &fileLink,const QString &filename):"<<fileLink <<filename;
//    QString fileLink = name.trimmed();
    const QUrl fileUrl = QUrl::fromUserInput(fileLink);//处理用户输入的 URL 字符串,做一些标准化和错误检查的工作:
    qDebug() << !fileUrl.isValid() <<!REGEX_IS_LINK.match(fileLink).hasMatch();
    // 定义一个正则表达式模式，用于匹配网络下载链接 Qt::CaseSensitive区分大小写
    if(!fileUrl.isValid() || !REGEX_IS_LINK.match(fileLink).hasMatch()){
        QMessageBox::information(nullptr,tr("Error"),tr("Invalid URL: %1 :%2").arg(fileLink,fileUrl.errorString()));
        return;
    }

    if(QFile::exists(filename)){
        QString alreadyExists = "/data/"
                                    ? tr("There already exists a file called %1. Overwrite?")
                                    : tr("There already exists a file called %1 in the current directory. "
                                         "Overwrite?");
        QMessageBox::StandardButton response = QMessageBox::question(nullptr,
                                                                     tr("Overwrite Existing File"),
                                                                     alreadyExists.arg(QDir::toNativeSeparators(filename)),
                                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (response == QMessageBox::No)
            return;
        QFile::remove(filename);
    }
    // 检查路径是否存在
    QDir dir(QFileInfo(filename).path());
    if(!dir.exists()) {
        dir.mkpath(dir.path());
    }
    file = std::make_unique<QFile>(filename);
    if(!file->open(QIODevice::WriteOnly)){
        QMessageBox::information(nullptr,tr("错误"),tr("不支持保存操作，文件: %1 :%2").arg(QDir::toNativeSeparators(filename),file->errorString()));
        return;
    }

    ui->label_fileName->setText(fileLink);

    startRequest(fileUrl);
}


void FilesUtil::setRootUrl(const QString &url)
{
    qDebug()<<"NetworkFileModel::setRootUrl:"<<url;

}

void FilesUtil::startRequest(const QUrl &requestedUrl)
{
    qDebug()<<"startRequest(const QUrl &"<< requestedUrl;
    url = requestedUrl;
    httpRequestAborted = false;
    networkReply.reset(m_networkAccessManager->get(QNetworkRequest(url)));
    //! [connecting-reply-to-slots]
    connect(networkReply.get(),&QNetworkReply::finished,this,&FilesUtil::NetworkReplyFinished);
    connect(networkReply.get(), &QIODevice::readyRead, this, &FilesUtil::NetworkReplyReadyRead);
    connect(networkReply.get(),&QNetworkReply::downloadProgress,this,&FilesUtil::NetworkReplyDownloadProgress);
    //! [networkreply-readyread-1]
}

void FilesUtil::cancelRequest()
{
    qDebug()<<"NetworkFileModel::cancelRequest()";
    httpRequestAborted = true;
    networkReply->abort();
}

void FilesUtil::readAllFilesFromLocalPath(const QString &directory)
{
    QDir dir(directory);

    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files)) {
//        qDebug()<<"fileInfo.fileName()"<<fileInfo.fileName();
//        qDebug()<<"fileInfo.absoluteFilePath"<<fileInfo.absoluteFilePath()<<fileInfo.absoluteFilePath().remove(0,directory.length());
//        qDebug()<<"fileInfo.Path"<<fileInfo.path() <<directory.length()<<fileInfo.path().remove(0,directory.length());
        filesListLocal << fileInfo.absoluteFilePath();
//        filesListLocal.append(fileInfo.absoluteFilePath());
    }
    foreach(QFileInfo dirInfo, dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        readAllFilesFromLocalPath(dirInfo.absoluteFilePath());
    }
}

QStringList FilesUtil::getFilesListLocalPath(const QString &path)
{
    filesListLocal.clear();

    readAllFilesFromLocalPath(path);

    return filesListLocal;
}

QStringList FilesUtil::getFilesListNetworkPath(const QString &url)
{
    qDebug()<<"FilesUtil::getFilesListNetworkPath(const QString &url)"<<url;
    filesListNetwork.clear();
    parseFilesListNetworkPath(url);
    return filesListNetwork;
}

void FilesUtil::parseFilesListNetworkPath(const QString &url)
{
    qDebug()<<"parseFilesListNetworkPath(const QString &url)"<<url<<QUrl(url);
    QNetworkAccessManager manager;
    // 获取页面内容
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if(reply->error() != QNetworkReply::NoError) {
        // 抛出/返回错误
        qDebug() << "Error occurred:" << reply->errorString();
        QMessageBox::warning(this, "网络请求错误", "网络请求获取文件下载链接:"+url+". 信息:"+reply->errorString());
        return;
    }

    QString pageSource = reply->readAll();
    qDebug()<<"pageSource:"<<pageSource;
    QList<QString> links;

    // 匹配所有项
    QRegularExpressionMatchIterator matches = REGEX_LINKS.globalMatch(pageSource);
    while (matches.hasNext()) {
        links << url+matches.next().captured(1);
    }
    qDebug()<<"QList<QString>:"<< links;
    foreach(QString path, links) {
        qDebug()<<path.lastIndexOf(".")<<path.lastIndexOf("/"); //-1 对应失败
        if (path.contains(".") && path.lastIndexOf(".") > path.lastIndexOf("/")) {
            qDebug() << "This is a file."<<path;
            // 处理文件类型
            filesListNetwork <<path;
        } else {
            qDebug() << "This is a directory."<<path;
            // 处理路径类型
            parseFilesListNetworkPath(path);
        }
    }

}

bool FilesUtil::downloadFileFromNetworkLink(const QUrl &link)
{
    qDebug()<<"downloadFileFromNetworkLink(const QString &link)"<<link;
    // 定义一个正则表达式模式，用于匹配网络下载链接 Qt::CaseSensitive区分大小写
//    qDebug() << REGEX_IS_LINK.match(link).hasMatch();
//    if(!REGEX_IS_LINK.match(link).hasMatch()){
//        qDebug() << "This is not a isSymLink()"<<link;
//        return false;
//    }

    startRequest(link);
    return true;

}

void FilesUtil::downloadFile(const QString &url)
{
    qDebug()<<"NetworkFileModel::downloadFile"<<url<<this->url;
    if(url.isEmpty()){return;}

    const QUrl fileUrl = QUrl::fromUserInput(url);
    if(!fileUrl.isValid()){
        QMessageBox::information(nullptr,tr("Error"),tr("Invalid URL: %1 :%2").arg(url,fileUrl.errorString()));
        return;
    }

    QString fileName = fileUrl.fileName();
    if (fileName.isEmpty()){
        fileName = "";

    }
    if (QFile::exists(fileName)) {
        QString alreadyExists = "/data/"
                ? tr("There already exists a file called %1. Overwrite?")
                : tr("There already exists a file called %1 in the current directory. "
                     "Overwrite?");
        QMessageBox::StandardButton response = QMessageBox::question(nullptr,
                tr("Overwrite Existing File"),
                alreadyExists.arg(QDir::toNativeSeparators(fileName)),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (response == QMessageBox::No)
            return;
        QFile::remove(fileName);
    }

    // file = openFileForWrite(fileName);
    // if (!file)
        // return;

    // downloadButton->setEnabled(false);

    // schedule the request
    startRequest(fileUrl);

}

void FilesUtil::initNetwork()
{
    qDebug()<<"NetworkFileModel::initNetwork()";

    m_networkAccessManager = new QNetworkAccessManager;

//    m_networkReply = new QNetworkReply;

}

void FilesUtil::NetworkReplyFinished()
{
    qDebug()<<"NetworkReplyFinished()";
    QFileInfo fi;
    if(file){
        fi.setFile(file->fileName());
        file->close();
        file.reset();
    }

    //! [networkreply-error-handling-1]
    QNetworkReply::NetworkError error = networkReply->error();
    const QString &errorString = networkReply->errorString();
    //! [networkreply-error-handling-1]
    networkReply.reset();
    //! [networkreply-error-handling-2]
    if(error != QNetworkReply::NoError){
        QFile::remove(fi.absoluteFilePath());
        // For "request aborted" we handle the label and button in cancelDownload()
        if(!httpRequestAborted){
            qDebug()<<tr("Download failed:\n%1.").arg(errorString);
            ui->label_fileName->setText(tr("文件：%1 下载失败:\n%2.").arg(fi.fileName()).arg(errorString));
            //下载开放按钮
            ui->checkBox_downloadFinished->setCheckState(Qt::Unchecked);
        }
        return;
    }
    //! [networkreply-error-handling-2]

    ui->label_fileName->setText(tr("Downloaded %1 bytes to %2\nin\n%3")
                                 .arg(fi.size())
                                 .arg(fi.fileName(), QDir::toNativeSeparators(fi.absolutePath())));
    //下载完成
    ui->checkBox_downloadFinished->setCheckState(Qt::Checked);
}

//! [networkreply-readyread-2]
void FilesUtil::NetworkReplyReadyRead()
{
    qDebug()<<"NetworkReplyReadyRead()";
    // 打印响应头
    QList<QNetworkReply::RawHeaderPair> headers = networkReply->rawHeaderPairs();
    qDebug() << "Ready Read:" <<headers;
    foreach (const QNetworkReply::RawHeaderPair &header, headers) {
        qDebug() <<"Ready Read:"<< header.first << ": " << header.second;
//        printf("%s: %s\n", qPrintable(header.first), qPrintable(header.second));
    }
    qDebug() << "Content-Type:" << networkReply->header(QNetworkRequest::ContentTypeHeader);
    // 或者直接打印个别重要属性
    qDebug() << "Status code:" << networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // 读取响应数据
    QString contentType = networkReply->header(QNetworkRequest::ContentTypeHeader).toString();

    QByteArray responseData = networkReply->readAll();

    if(contentType.contains("text/plain")) {
        qDebug() << "普通纯文本数据,没有任何格式信息,可以直接显示或保存";
        file->write(responseData);
        file->flush();
        qDebug()<<"file->write(networkReply->readAll())";
    } else if(contentType.contains("text/html")) {
        qDebug() << "HTML文件,可以直接显示或者解析为DOM树";
    } else if(contentType.contains("image/jpeg")) {
        qDebug() << "图片文件,可以直接显示或保存";
    } else if(contentType.contains("application/json")) {
        qDebug() << "JSON数据,需要用JSON解析器解析";
    } else {
        qDebug() << "其他文件类型如文本、视频等需要特殊处理";
    }

}
//! [networkreply-readyread-2]

void FilesUtil::NetworkReplyDownloadProgress(int bytesReceived, int bytesTotal)
{
    qDebug() << "Downloaded进度：" << bytesReceived << "of" << bytesTotal;
    ui->progressBar_Download->setMaximum(bytesTotal);
    ui->progressBar_Download->setValue(bytesReceived);

}

void FilesUtil::on_checkBox_downloadFinished_stateChanged(int arg1)
{
    qDebug() << "on_checkBox_downloadFinished_stateChanged(int arg1)" <<Qt::Checked<< arg1<<(arg1 == Qt::Checked);
    bool flag = arg1 == Qt::Checked;
    ui->pushButton_openFromLocalFile->setVisible(flag);
    ui->pushButton_cancelDownload->setVisible(!flag);
    ui->pushButton_pauseDownload->setVisible(!flag);
}


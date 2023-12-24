#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QRegularExpression>   // 正则表达式
#include <QStandardItemModel>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QEventLoop>
#include <QTreeView>
#include <QFile>
#include <QDir>
#include <QXmlStreamReader>

#include <iostream>
using namespace std;


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void AnalysisHTML(QString path);

    QString HtmlResponse;
    QString testline;
private slots:
    void on_pushButton_clicked();
    void replyFinished(QNetworkReply *);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H

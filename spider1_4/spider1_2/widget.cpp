#include "widget.h"
#include "ui_widget.h"
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>

void writeToFile(QString &filePath, QString &data) {
    // 打开文件
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    // 创建文本流
    QTextStream out(&file);

    // 写入数据
    out << data; // 使用 endl 添加换行符

    // 关闭文件
    file.close();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString url_string = ui->URL_lineEdit->text();
    QUrl url=QUrl(url_string);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);

    //设置Headers的信息
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    QString cookie_string = ui->cookie_lineEdit->text();
    request.setRawHeader("Cookie", cookie_string.toUtf8());
    request.setRawHeader("Sec-Ch-Ua-Platform", "Windows");

    connect(manager,SIGNAL(finished(QNetworkReply *)),this,
            SLOT(replyFinished(QNetworkReply *)));

    manager->get(request);  // 发送请求

}

void Widget::replyFinished(QNetworkReply *reply){
    HtmlResponse.clear();
    testline.clear();
    while (!reply->atEnd())
        HtmlResponse+=reply->readAll();
    reply->deleteLater();
    qDebug()<<HtmlResponse;

    QString test = "D:/A_second_term/spider1_2/test.txt";

    //writeToFile(test,HtmlResponse);

    AnalysisHTML(test);

}


void Widget::AnalysisHTML(QString path) {

    /*-------------------存储破译后整理好的信息------------------*/
    QFile file("D:/A_second_term/spider1_2/data.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    // 创建文本流
    QTextStream out(&file);

    out<<"{\"jobList\":[";

    /*-------------------用于初始破译HTML------------------*/

    QFile file1(path);
    if (!file1.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening file: " << file1.errorString();
    }
    QTextStream in(&file1);
    while (!in.atEnd()) {
        testline += in.readLine();
    }

    file1.close();

    //文件转为json格式数据
    QJsonDocument jsonDoc = QJsonDocument::fromJson(testline.toUtf8());
    if (!jsonDoc.isNull()) {
        // 如果解析成功，将QJsonDocument转换为QJsonObject
        if (jsonDoc.isObject()) {
            QJsonObject jsonObject = jsonDoc.object();

            // 读取JSON对象中的值
            for(int i=0;i<29;i++){

                //第一层解码
                QJsonObject obj1=jsonObject["zpData"].toObject();
                QJsonArray arr1 =obj1["jobList"].toArray();

                //第二层解码
                QJsonObject obj2=arr1[i].toObject();

                /*--------------------------第三层信息彻底解码--------------------------*/
                //工作名称
                QString name=obj2["jobName"].toString();
                //工作经验要求
                QString exp=obj2["jobExperience"].toString();
                //工作要求能力（多个）
                QJsonArray arr2 =obj2["skills"].toArray();
                QList<QString> skills;
                for (const QJsonValue &value : arr2) {
                    skills<<value.toString();
                }
                //工作薪资
                QString salary=obj2["salaryDesc"].toString();
                //基本待遇（多个）
                QList<QString> welfare;
                QJsonArray arr3=obj2["welfareList"].toArray();
                for (const QJsonValue &value : arr3) {
                    welfare<<value.toString();
                }

                // 输出解析结果
                qDebug()<<"----------------------"<<"工作"<<i<<"----------------------";
                qDebug() <<"工作名称"<<i<<":"<<name;
                qDebug() <<"经验要求"<<i<<":"<<exp;
                qDebug() <<"要求能力"<<i<<":"<<skills;
                qDebug() <<"工作薪资"<<i<<":"<<salary;
                qDebug() <<"基本待遇"<<i<<":"<<welfare;

                /*---------------------------将数据整理后写入文件--------------------------*/
                //整理数据,由于out函数的性质，只能逐条进行
                out<<"{\"name\": \"";
                out<<name;
                out<<'"';
                out<<',';

                out<<"\"exp\": \"";
                out<<exp;
                out<<'"';
                out<<',';

                out<<"\"skills\": [";
                for(int m=0; m<skills.count(); m++) {
                    out<<'"';
                    out<<skills.at(m);
                    out<<'"';
                    if(m != skills.count()-1) {
                        out<<',';
                    }
                }
                out<<']';
                out<<',';

                out<<"\"salary\": \"";
                out<<salary;
                out<<'"';
                out<<',';

                out<<"\"welfare\": [";
                for(int n=0; n<welfare.count(); n++) {
                    out<<'"';
                    out<<welfare.at(n);
                    out<<'"';
                    if(n != welfare.count()-1) {
                        out<<',';
                    }
                }
                out<<']';

                out<<'}';

            }
            out<<'}';
        } else {
            qDebug() << "JSON document is not an object";
        }
    } else {
        qDebug() << "Invalid JSON";
    }
    // 关闭文件
    file.close();

}


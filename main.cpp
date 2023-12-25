#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file("E:/qtproject/testQt/data.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open the file for reading.";
        return -1;
    }

    QString data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON Parse Error:" << parseError.errorString();
        return -1;
    }

    // 确保读取到的JSON数据是一个对象，并且该对象包含键"jobList"
    if (!document.isObject() || !document.object().contains("jobList")) {
        qDebug() << "The JSON document is not an object or does not contain 'jobList'";
        return -1;
    }

    QJsonArray jsonArray = document.object().value("jobList").toArray();


    // 保存到QJsonObject数组中
    QList<QJsonObject> jsonObjectList;
    for (const QJsonValue &value : jsonArray) {
         if (value.isObject()) {
             jsonObjectList.append(value.toObject());
         }
     }


    // 测试输出
//   for (const QJsonObject &obj : jsonObjectList) {
//       qDebug() << "Name:" << obj.value("name").toString();
//       qDebug() << "Experience:" << obj.value("exp").toString();
//       qDebug() << "Skills:" << obj.value("skills").toArray().toVariantList();
//       qDebug() << "Salary:" << obj.value("salary").toString();
//       qDebug() << "Welfare:" << obj.value("welfare").toArray().toVariantList();
//       qDebug() << "----------";
//   }


    MainWindow w;
    w.show();
    return a.exec();
}

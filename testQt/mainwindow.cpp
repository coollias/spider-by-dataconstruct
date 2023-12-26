#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSqlDatabase"
#include "QSqlError"
#include "QDebug"
#include "QTableWidget"
#include "QSqlQuery"
#include "QSqlRecord"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "QString"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QTextStream>
#include <QThreadPool>
#include <QRegularExpression>

// 构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 数据库唯一实例
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/kk/Desktop/testQt/Infomation.db");

    if(db.open()){
        qDebug()<<"连接成功";
    }else{
        qDebug()<<"失败";
    }
}


// 析构函数
MainWindow::~MainWindow()
{
    delete ui;
}


// 展示所有数据
void MainWindow::on_btnquery_clicked()
{
    Query();
}


// 查询函数
void MainWindow::Query()
{
    int row = 0;
    ui->tableWidget->setRowCount(0);

    QSqlQuery query;
    if(query.exec("SELECT * FROM Job")){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salart_max","skills","welfare"});
        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row,var,new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug()<<"查询失败："<<query.lastError().text();
    }
}


// 数据导入数据库
void MainWindow::on_btnInDB_clicked()
{
    //打开文件
    QFile file("C:/Users/kk/Desktop/testQt/data.txt");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("无法打开文件！");
        return;
    }

    //读取文件内容并解析为JSON
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        qWarning("JSON格式错误！");
        return;
    }

    QJsonObject jsonObj = doc.object();
    QJsonArray jobList = jsonObj["jobList"].toArray();

    // 执行sql语句的实例
    QSqlQuery query;
    // 插入数据前先清空旧数据
    query.exec("DELETE FROM Job");

    // 遍历职位数组
    for (const QJsonValue &value : jobList) {
        QJsonObject jobObj = value.toObject();
        QString name = jobObj["name"].toString();
        QString exp = jobObj["exp"].toString();
        QString salary = jobObj["salary"].toString();
        QJsonArray skillsArray = jobObj["skills"].toArray();
        QJsonArray welfareArray = jobObj["welfare"].toArray();

        // 解析工作经验
        QStringList expSplit = exp.split('-');
        int exp_min = expSplit[0].toInt();
        int exp_max = expSplit.size() > 1 ? expSplit[1].replace("年", "").toInt() : 100;

        // 解析薪资
        QRegularExpression re(R"(([0-9]+)-?([0-9]*)K?.*)");
        QRegularExpressionMatch match = re.match(salary);
        QStringList salarySplit;

        if (match.hasMatch()) {
            salarySplit << match.captured(1); // 捕获最小薪资
            if (!match.captured(2).isEmpty()) {
                salarySplit << match.captured(2); // 捕获最大薪资（如果有）
            }
        }
        float salary_min = 0.0f;
        float salary_max = 0.0f;

        if (!salarySplit.isEmpty()) {
            salary_min = salarySplit[0].toFloat(); // 转换最小薪资
        }

        if (salarySplit.size() > 1) {
            salary_max = salarySplit[1].toFloat(); // 转换最大薪资
        }

        // 转换技能为字符串
        QStringList skillsList;
        for (const QJsonValue &skill : skillsArray) {
            skillsList.append(skill.toString());
        }
        QString skills = skillsList.join(",");

        // 转换福利为字符串
        QStringList welfareList;
        for (const QJsonValue &welfare : welfareArray) {
            welfareList.append(welfare.toString());
        }
        QString welfare = welfareList.join(",");


        // 插入数据到数据库
        query.prepare("INSERT INTO Job (name, exp_min, exp_max, salary_min, salary_max, skills, welfare) "
                              "VALUES (:name, :exp_min, :exp_max, :salary_min, :salary_max, :skills, :welfare)");
        query.bindValue(":name", name);
        query.bindValue(":exp_min", exp_min);
        query.bindValue(":exp_max", exp_max);
        query.bindValue(":salary_min", salary_min);
        query.bindValue(":salary_max", salary_max);
        query.bindValue(":skills", skills);
        query.bindValue(":welfare", welfare);

        if (!query.exec()) {
            qWarning() << "插入数据失败：" << query.lastError().text();
        }
    }
}


// 薪资查询
void MainWindow::on_btnSalaryQuery_clicked()
{
    QString inputLow = ui->salary_lower->text();// 薪资下限
    int low;
    if(inputLow.isEmpty()){
        low = 0;
    }else{
        low = inputLow.toInt();
    }

    QString inputUp = ui->salary_upper->text(); // 薪资上限
    int up;
    if(inputUp.isEmpty()){
        up = 1e9;
    }else{
        up = inputUp.toInt();
    }

    // 执行查询
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job WHERE salary_min >= :low AND salary_max <= :up");
    query.prepare(queryString);
    query.bindValue(":low", low);
    query.bindValue(":up", up);


    // 展示查询结果
    int row = 0;
    ui->tableWidget->setRowCount(0);

    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }

}


// 工作年限查询
void MainWindow::on_btnExpQuery_clicked()
{
    QString inputLow = ui->exp_lower->text();// 年限下限
    int low;
    if(inputLow.isEmpty()){
        low = 0;
    }else{
        low = inputLow.toInt();
    }

    QString inputUp = ui->exp_upper->text(); // 年限上限
    int up;
    if(inputUp.isEmpty()){
        up = 1e9;
    }else{
        up = inputUp.toInt();
    }

    // 执行查询
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job WHERE exp_min >= :low AND exp_max <= :up or exp_min = 0 AND exp_max = 100");
    query.prepare(queryString);
    query.bindValue(":low", low);
    query.bindValue(":up", up);


    // 展示查询结果
    int row = 0;
    ui->tableWidget->setRowCount(0);

    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}


// 职位查询（模糊匹配）
void MainWindow::on_btnNameQuery_clicked()
{
    QString job_str = ui->jobName->text().trimmed(); // 获取用户输入

    // 执行查询
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job WHERE name LIKE '%%1%' COLLATE NOCASE").arg(job_str); // 模糊匹配
    query.prepare(queryString);

    // 展示查询结果
    int row = 0;
    ui->tableWidget->setRowCount(0);
    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}


// 爬虫函数
void MainWindow::on_btnspider_clicked()
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


// HTML文件解析
void MainWindow::AnalysisHTML(QString res)
{

    /*-------------------存储破译后整理好的信息------------------*/
    QFile file("C:/Users/kk/Desktop/testQt/test.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    // 创建文本流
    QTextStream out(&file);

    out<<"{\"jobList\":[";

    testline=res;
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


// 写入文件
void writeToFile(QString &filePath, QString &data) {
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    // 创建文本流
    QTextStream out(&file);
    out << data;

    file.close();
}


// 处理网络请求的回复
void MainWindow::replyFinished(QNetworkReply * reply)
{
    HtmlResponse.clear();
    testline.clear();
    while (!reply->atEnd())
        HtmlResponse+=reply->readAll();
    reply->deleteLater();
    qDebug()<<HtmlResponse;

    QString test = "test.txt";

    writeToFile(test,HtmlResponse);

    AnalysisHTML(HtmlResponse);
}


// 薪资上限降序
void MainWindow::on_btnSalarySort_clicked()
{
    // 执行排序
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job ORDER BY salary_max DESC");
    query.prepare(queryString);

    // 展示排序结果
    int row = 0;
    ui->tableWidget->setRowCount(0);
    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}


// 年限下限升序
void MainWindow::on_btnExpSort_clicked()
{
    // 执行排序
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job ORDER BY exp_min ASC");
    query.prepare(queryString);

    // 展示排序结果
    int row = 0;
    ui->tableWidget->setRowCount(0);
    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}


// 薪资和年限共同查询
void MainWindow::on_btnExpSalaryQuery_clicked()
{
    QString salary_low = ui->salary_lower->text();// 薪资下限
    int s_low;
    if(salary_low.isEmpty()){
        s_low = 0;
    }else{
        s_low = salary_low.toInt();
    }

    QString salary_up = ui->salary_upper->text(); // 薪资上限
    int s_up;
    if(salary_up.isEmpty()){
        s_up = 1e9;
    }else{
        s_up = salary_up.toInt();
    }

    QString exp_low = ui->exp_lower->text();// 年限下限
    int e_low;
    if(exp_low.isEmpty()){
        e_low = 0;
    }else{
        e_low = exp_low.toInt();
    }

    QString exp_up = ui->exp_upper->text(); // 年限上限
    int e_up;
    if(exp_up.isEmpty()){
        e_up = 1e9;
    }else{
        e_up = exp_up.toInt();
    }

    // 执行查询
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job WHERE ((exp_min >= :elow AND exp_max <= :eup) OR (exp_min = 0 AND exp_max = 100)) AND (salary_min >= :slow AND salary_max <= :sup)");
    query.prepare(queryString);
    query.bindValue(":elow", e_low);
    query.bindValue(":eup", e_up);
    query.bindValue(":slow", s_low);
    query.bindValue(":sup", s_up);

    // 展示查询结果
    int row = 0;
    ui->tableWidget->setRowCount(0);

    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}


// 技能查询（模糊匹配）
void MainWindow::on_btnSkillQuery_clicked()
{
    QString skill_str = ui->skill->text().trimmed(); // 获取用户输入

    // 执行查询
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job WHERE skills LIKE '%%1%' COLLATE NOCASE").arg(skill_str); // 模糊匹配
    query.prepare(queryString);

    // 展示查询结果
    int row = 0;
    ui->tableWidget->setRowCount(0);
    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}


// 福利查询（模糊匹配）
void MainWindow::on_btnWelfareQuery_clicked()
{
    QString welfare_str = ui->skill->text().trimmed(); // 获取用户输入

    // 执行查询
    QSqlQuery query;
    QString queryString = QString("SELECT * FROM Job WHERE welfare LIKE '%%1%' COLLATE NOCASE").arg(welfare_str); // 模糊匹配
    query.prepare(queryString);

    // 展示查询结果
    int row = 0;
    ui->tableWidget->setRowCount(0);
    if(query.exec()){
        QSqlRecord record = query.record();
        ui->tableWidget->setColumnCount(record.count());
        ui->tableWidget->setHorizontalHeaderLabels({"id","name","exp_min","exp_max","salary_min","salary_max","skills","welfare"});

        while (query.next()){
            ui->tableWidget->insertRow(row);
            for (int var = 0; var < record.count(); ++var) {
                ui->tableWidget->setItem(row, var, new QTableWidgetItem(query.value(var).toString()));
            }
            row++;
        }
    }else{
        qDebug() << "查询失败：" << query.lastError().text();
    }
}

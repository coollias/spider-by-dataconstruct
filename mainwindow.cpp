#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QSqlDatabase"
#include "QDebug"
#include "QSqlError"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 数据库实例
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

//    QStringList list = QSqlDatabase::drivers();
//    qDebug() << list;

    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("1234");
    db.setDatabaseName("db01");
    db.setPort(3306);

    if(!db.open()){
        qDebug()<<"失败："<<db.lastError().text();
    }else{
        qDebug()<<"成功";
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}


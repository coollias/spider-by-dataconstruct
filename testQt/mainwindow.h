#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QSqlQuery"
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QStandardItemModel>
#include <QtNetwork/QNetworkReply>
#include <QMessageBox>
#include <QEventLoop>
#include <QTreeView>
#include <QFile>
#include <QDir>
#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Query();

    void AnalysisHTML(QString path);

    QString HtmlResponse;
    QString testline;

private slots:
    void on_btnquery_clicked();

    void on_btnInDB_clicked();

    void on_btnSalaryQuery_clicked();

    void on_btnExpQuery_clicked();

    void on_btnNameQuery_clicked();

    void on_btnspider_clicked();

    void replyFinished(QNetworkReply *);

    void on_btnSalarySort_clicked();

    void on_btnExpSort_clicked();

    void on_btnExpSalaryQuery_clicked();

    void on_btnSkillQuery_clicked();

    void on_btnWelfareQuery_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

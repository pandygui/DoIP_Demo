#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "DoIP_pc.h"
#include "mytest.h"
#include <QObject>
#include <stdio.h>
#include <QCloseEvent>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString str2qstrA(const std::string str);
    std::string qstr2strA(const QString qstr);

    //init display
    void mydisplay();

    //case1
    void case1_display1();
    void case1_display2();
    void case1_display3();

    //case2
    void case2_display1();
    void case2_display2();

    //case3
    void case3_display1();
    void case3_display2();

    //case4
    void case4_display1();
    void case4_display2();
    void case4_display3();



private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    //void mydisplay();



protected:
    void colseEvent(QCloseEvent* event);




public:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H

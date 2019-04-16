#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string.h>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("车载系统");
    setWindowIcon(QIcon(":/image/1.jpg"));

    QPalette palette;//创建一个调色板对象
    palette.setBrush(this->backgroundRole(),QPixmap(":/image/1.jpg"));//设置背景图片
    this->setPalette(palette);//应用这个调色板

    QFont ft;
    ft.setPointSize(22);
    ui->label->setFont(ft);
    ui->pushButton_5->setEnabled(false);


#if 1
    connect(MT::instance(), &MT::mysignals1, this, &MainWindow::mydisplay);

    connect(MT::instance(), &MT::mysignals2, this, &MainWindow::case1_display1);
    connect(MT::instance(), &MT::mysignals3, this, &MainWindow::case1_display2);
    connect(MT::instance(), &MT::mysignals4, this, &MainWindow::case1_display3);

    //connect(MT::instance(), &MT::mysignals22, this, &MainWindow::case2_display1);
    connect(MT::instance(), &MT::mysignals5, this, &MainWindow::case2_display2);

    connect(MT::instance(), &MT::mysignals6, this, &MainWindow::case3_display1);
    connect(MT::instance(), &MT::mysignals7, this, &MainWindow::case3_display2);

    connect(MT::instance(), &MT::mysignals8, this, &MainWindow::case4_display1);
    connect(MT::instance(), &MT::mysignals9, this, &MainWindow::case4_display2);
    connect(MT::instance(), &MT::mysignals10, this, &MainWindow::case4_display3);

#endif

    init();

#if 0
    ui->textEdit->append("Init detection equipment start");
    mydisplay();
#endif








}

MainWindow::~MainWindow()
{
    delete ui;
}


//button1
void MainWindow::on_pushButton_clicked()
{
    requestVehicleIdentification();

#if 0
    if(MT::signal2 == 1)
    {
        case1_display1();
    }
    if(MT::signal3 == 1)
    {
        case1_display2();
    }
    if(MT::signal4 == 1)
    {
        case1_display3();
    }
    MT::signal2 = 0;
    MT::signal3 = 0;
    MT::signal4 = 0;
#endif



}

//button2
void MainWindow::on_pushButton_2_clicked()
{
    requestEntityStatus();
    if(MT::signal22 == 1)
    {
        case2_display1();
    }
    MT::signal22 = 0;

#if 0
    if(MT::signal22 == 1)
    {
        case2_display1();
    }
    if(MT::signal5 == 1)
    {
        case2_display2();
    }
    MT::signal22 = 0;
    MT::signal5 = 0;
#endif
}

//button3
void MainWindow::on_pushButton_3_clicked()
{
    requestRoutingActivation();

#if 0
    if(MT::signal6 == 1)
    {
        case3_display1();
    }
    if(MT::signal7 == 1)
    {
        case3_display2();
    }
    MT::signal6 = 0;
    MT::signal7 = 0;
 #endif
}

//button4
void MainWindow::on_pushButton_4_clicked()
{

    sendDiagnosticMessage();

#if 0
    if(MT::signal9 == 1)
    {
        case4_display2();

    }
    if(MT::signal10 == 1)
    {
        case4_display3();
    }
    if(MT::signal8 == 1)
    {
        case4_display1();
    }
    MT::signal8 = 0;
    MT::signal9 = 0;
    MT::signal10 = 0;

#endif

}


//deinit close
void MainWindow::colseEvent(QCloseEvent * event)
{
    Q_UNUSED(event);
    deinit();
}


//case1
void MainWindow::case1_display1()
{
    ui->textEdit->append("*************************** Vehicle Identification Request ***************************");
    //
    QString str1 = str2qstrA(MT::case1_1.srcIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case1_1.srcPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   ----->    ");


    QString str3 = str2qstrA(MT::case1_1.dstIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case1_1.dstPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case1_2.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case1_2.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case1_2.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case1_2.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");




}

void MainWindow::case1_display2()
{
    ui->textEdit->append("*************************** Vehicle Identification Response ***************************");
    //
    QString str1 = str2qstrA(MT::case1_3.dstIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case1_3.dstPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   <-----    ");


    QString str3 = str2qstrA(MT::case1_3.srcIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case1_3.srcPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case1_4.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case1_4.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case1_4.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case1_4.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Payload Info List : ");
    QString str14 = str2qstrA(MT::case1_5.vin);
    ui->textEdit->append("VIN : ");
    ui->textEdit->insertPlainText(str14);

    QString str9= str2qstrA(MT::case1_5.eid);
    ui->textEdit->append("EID : ");
    ui->textEdit->insertPlainText(str9);

    QString str12 = str2qstrA(MT::case1_5.gid);
    ui->textEdit->append("GID :");
    ui->textEdit->insertPlainText(str12);

    QString str10 = str2qstrA(MT::case1_5.entityLA);
    ui->textEdit->append("Entity Logical Address : ");
    ui->textEdit->insertPlainText(str10);

    QString str11 = str2qstrA(MT::case1_5.furtherAction);
    ui->textEdit->append("Further Action : ");
    ui->textEdit->insertPlainText(str11);

    QString str13= str2qstrA(MT::case1_5.syncStatus);
    ui->textEdit->append("Sync Status : ");
    ui->textEdit->insertPlainText(str13);

}

void MainWindow::case1_display3()
{
    ui->textEdit->append("*************************** Announcement Identification Response ***************************");
    //
    QString str1 = str2qstrA(MT::case1_6.dstIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case1_6.dstPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   <-----    ");


    QString str3 = str2qstrA(MT::case1_6.srcIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case1_6.srcPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case1_7.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case1_7.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case1_7.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case1_7.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Payload Info List : ");
    QString str14 = str2qstrA(MT::case1_8.vin);
    ui->textEdit->append("VIN : ");
    ui->textEdit->insertPlainText(str14);

    QString str9= str2qstrA(MT::case1_8.eid);
    ui->textEdit->append("EID : ");
    ui->textEdit->insertPlainText(str9);

    QString str12 = str2qstrA(MT::case1_8.gid);
    ui->textEdit->append("GID :");
    ui->textEdit->insertPlainText(str12);

    QString str10 = str2qstrA(MT::case1_8.entityLA);
    ui->textEdit->append("Entity Logical Address : ");
    ui->textEdit->insertPlainText(str10);

    QString str11 = str2qstrA(MT::case1_8.furtherAction);
    ui->textEdit->append("Further Action : ");
    ui->textEdit->insertPlainText(str11);

    QString str13= str2qstrA(MT::case1_8.syncStatus);
    ui->textEdit->append("Sync Status : ");
    ui->textEdit->insertPlainText(str13);
}

//case2
void MainWindow::case2_display1()
{

    ui->textEdit->append("*************************** Entity Status Request ***************************");
    //
    QString str1 = str2qstrA(MT::case2_1.srcIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case2_1.srcPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   ----->    ");


    QString str3 = str2qstrA(MT::case2_1.dstIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case2_1.dstPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case2_2.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case2_2.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case2_2.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case2_2.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");
}

void MainWindow::case2_display2()
{
    ui->textEdit->append("*************************** Entity Status Response ***************************");
    //send
    QString str1 = str2qstrA(MT::case2_3.dstIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case2_3.dstPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   <-----    ");


    QString str3 = str2qstrA(MT::case2_3.srcIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case2_3.srcPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //header
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case2_4.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case2_4.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case2_4.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case2_4.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");

    //payload
    ui->textEdit->append("DoIP Message Payload Info List : ");
    QString str14 = str2qstrA(MT::case2_5.nodeType);
    ui->textEdit->append("Node Type : ");
    ui->textEdit->insertPlainText(str14);

    QString str9= str2qstrA(MT::case2_5.maxTcpSockets);
    ui->textEdit->append("Max Tcp Sockets : ");
    ui->textEdit->insertPlainText(str9);

    QString str12 = str2qstrA(MT::case2_5.currentTcpSockets);
    ui->textEdit->append("Current Tcp Sockets :");
    ui->textEdit->insertPlainText(str12);

    QString str10 = str2qstrA(MT::case2_5.maxDataSize);
    ui->textEdit->append("Max Data Size : ");
    ui->textEdit->insertPlainText(str10);

}


//case3
void MainWindow::case3_display1()
{
    //
    ui->textEdit->append("*************************** TCP Routing Activation Request ***************************");
    //
    QString str1 = str2qstrA(MT::case3_1.srcIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case3_1.srcPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   ----->    ");


    QString str3 = str2qstrA(MT::case3_1.dstIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case3_1.dstPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case3_2.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case3_2.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case3_2.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case3_2.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");


    //
    QString str9= str2qstrA(MT::case3_3.activationType);
    ui->textEdit->append("Activation Type : ");
    ui->textEdit->insertPlainText(str9);

    QString str10 = str2qstrA(MT::case3_3.externalLA);
    ui->textEdit->append("external LA : ");
    ui->textEdit->insertPlainText(str10);

}

void MainWindow::case3_display2()
{
    //
    ui->textEdit->append("*************************** TCP Routing Activation Response ***************************");
    //send
    QString str1 = str2qstrA(MT::case3_4.dstIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case3_4.dstPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   <-----    ");


    QString str3 = str2qstrA(MT::case3_4.srcIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case3_4.srcPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //header
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case3_5.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case3_5.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case3_5.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case3_5.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");

    //
    QString str9= str2qstrA(MT::case3_6.entityLA);
    ui->textEdit->append("Entity Logic address : ");
    ui->textEdit->insertPlainText(str9);

    QString str10 = str2qstrA(MT::case3_6.externalLA);
    ui->textEdit->append("External Logic address : ");
    ui->textEdit->insertPlainText(str10);

    QString str11 = str2qstrA(MT::case3_6.responseCode);
    ui->textEdit->append("Response Code : ");
    ui->textEdit->insertPlainText(str11);
}

//case4
void MainWindow::case4_display1()
{
    ui->textEdit->append("");
    QString str1= str2qstrA(MT::case4_1);
    ui->textEdit->append("");

    ui->textEdit->insertPlainText(str1);
}

void MainWindow::case4_display2()
{
    //
    ui->textEdit->append("*************************** Start Diagnostic Request ***************************");
    //
    QString str1 = str2qstrA(MT::case4_2.srcIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case4_2.srcPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   ----->    ");


    QString str3 = str2qstrA(MT::case4_2.dstIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case4_2.dstPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case4_3.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case4_3.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case4_3.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case4_3.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");

    //
    QString str9= str2qstrA(MT::case4_4.entityLA);
    ui->textEdit->append("Entity Logic address : ");
    ui->textEdit->insertPlainText(str9);

    QString str10 = str2qstrA(MT::case4_4.externalLA);
    ui->textEdit->append("External Logic address : ");
    ui->textEdit->insertPlainText(str10);

    QString str11 = str2qstrA(MT::case4_4.userData);
    ui->textEdit->append("User Data : ");
    ui->textEdit->insertPlainText(str11);
}

void MainWindow::case4_display3()
{
    //
    ui->textEdit->append("*************************** Diagnostic Response ***************************");
    QString str1 = str2qstrA(MT::case4_5.dstIp);
    ui->textEdit->append("Local IP : ");
    ui->textEdit->insertPlainText("Local Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str1);
    ui->textEdit->insertPlainText(" : ");
    QString str2 = str2qstrA(MT::case4_5.dstPort);
    ui->textEdit->insertPlainText(str2);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->insertPlainText("   <-----    ");


    QString str3 = str2qstrA(MT::case4_5.srcIp);
    ui->textEdit->insertPlainText("Remote IP : ");
    ui->textEdit->insertPlainText("Remote Port ");
    ui->textEdit->insertPlainText("[ ");
    ui->textEdit->insertPlainText(str3);
    ui->textEdit->insertPlainText(" : ");
    QString str4 = str2qstrA(MT::case4_5.srcPort);
    ui->textEdit->insertPlainText(str4);
    ui->textEdit->insertPlainText(" ]");
    ui->textEdit->append("");

    //header
    ui->textEdit->append("DoIP Message Header Info List : ");
    QString str8 = str2qstrA(MT::case4_6.protocolVersion);
    ui->textEdit->append("Protocol Version : ");
    ui->textEdit->insertPlainText(str8);

    QString str5 = str2qstrA(MT::case4_6.inverseProtocolVersion);
    ui->textEdit->append("Inverse Protocol Version : ");
    ui->textEdit->insertPlainText(str5);


    QString str6 = str2qstrA(MT::case4_6.payloadLength);
    ui->textEdit->append("Payload Length : ");
    ui->textEdit->insertPlainText(str6);

    QString str7 = str2qstrA(MT::case4_6.payloadType);
    ui->textEdit->append("Payload Type : ");
    ui->textEdit->insertPlainText(str7);
    ui->textEdit->append("");

    //
    QString str9= str2qstrA(MT::case4_7.ackCode);
    ui->textEdit->append("Ack Code : ");
    ui->textEdit->insertPlainText(str9);

    QString str10 = str2qstrA(MT::case4_7.entityLA);
    ui->textEdit->append("Entity Logic address : ");
    ui->textEdit->insertPlainText(str10);

    QString str11 = str2qstrA(MT::case4_7.externalLA);
    ui->textEdit->append("External Logic Address : ");
    ui->textEdit->insertPlainText(str11);

    QString str12 = str2qstrA(MT::case4_7.previousData);
    ui->textEdit->append("Previous Data : ");
    ui->textEdit->insertPlainText(str12);
}


//init display
void MainWindow::mydisplay()
{
    QString str1 = QString::fromStdString(MT::m_v1);
    ui->textEdit->append(str1);
    ui->textEdit->append("");

}


//string to Qstring me too
QString MainWindow::str2qstrA(const std::string str)
{
    return QString::fromLocal8Bit(str.data());
}

std::string MainWindow::qstr2strA(const QString qstr)
{
    QByteArray cdata = qstr.toLocal8Bit();
    return std::string(cdata);
}

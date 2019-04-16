#ifndef MYTEST_H
#define MYTEST_H

#include "DoIP_pc.h"
#include <QObject>
#include <stdio.h>

namespace UU {
class  MT;


}
class MT : public QObject
{
    Q_OBJECT
public:

    static int signal1;
    static int signal2;
    static int signal3;
    static int signal4;
    static int signal5;
    static int signal6;
    static int signal7;
    static int signal8;
    static int signal9;
    static int signal10;
    static int signal22;

    //init display
    static void show1(std::string v1);
    static std::string m_v1;

    //case1
    //1
    static void show2(SendInfoDisplay v1, HeaderDisplay v2);
    static SendInfoDisplay case1_1;
    static HeaderDisplay case1_2;

    static void show22(SendInfoDisplay v1, HeaderDisplay v2);
    static SendInfoDisplay case2_1;
    static HeaderDisplay case2_2;

    //2
    static void show3(SendInfoDisplay v1, HeaderDisplay v2, VehicleIdentificationDisplay v3);
    static SendInfoDisplay case1_3;
    static HeaderDisplay case1_4;
    static VehicleIdentificationDisplay case1_5;

    //3
    static void show4(SendInfoDisplay v1, HeaderDisplay v2, VehicleIdentificationDisplay v3);
    static SendInfoDisplay case1_6;
    static HeaderDisplay case1_7;
    static VehicleIdentificationDisplay case1_8;



    //case2
    //1
    static void show5(SendInfoDisplay v1, HeaderDisplay v2, EntityStatusDisplay v3);
    static SendInfoDisplay case2_3;
    static HeaderDisplay case2_4;
    static EntityStatusDisplay case2_5;


    //case_3
    //1
    static void show6(SendInfoDisplay v1, HeaderDisplay v2, RoutingActivationRequestDisplay v3);
    static SendInfoDisplay case3_1;
    static HeaderDisplay case3_2;
    static RoutingActivationRequestDisplay case3_3;

    //2
    static void show7(SendInfoDisplay v1, HeaderDisplay v2, RoutingActivationResponseDisplay v3);
    static SendInfoDisplay case3_4;
    static HeaderDisplay case3_5;
    static RoutingActivationResponseDisplay case3_6;


    //case4
    //1
    static void show8(std::string v1);
    static std::string case4_1;

    //2
    static void show9(SendInfoDisplay v1, HeaderDisplay v2, DiagnosticMessageDisplay v3);
    static SendInfoDisplay case4_2;
    static HeaderDisplay case4_3;
    static DiagnosticMessageDisplay case4_4;

    //3
    static void show10(SendInfoDisplay v1, HeaderDisplay v2, DiagnosticMessageAckDisplay v3);
    static SendInfoDisplay case4_5;
    static HeaderDisplay case4_6;
    static DiagnosticMessageAckDisplay case4_7;




public:
    //static MT::*ss;

    static MT* instance();
    static MT* MY;

signals:
     void mysignals1();
     void mysignals2();
     void mysignals3();
     void mysignals4();
     void mysignals5();
     void mysignals6();
     void mysignals7();
     void mysignals8();
     void mysignals9();
     void mysignals10();
     void mysignals22();



};
#endif // MYTEST_H

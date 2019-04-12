#include <stdio.h>
#include "mytest.h"
#include <unistd.h>
#include "mytest.h"


int MT::signal22 = 0;
int MT::signal1 = 0;
int MT::signal2 = 0;
int MT::signal3 = 0;
int MT::signal4 = 0;
int MT::signal5 = 0;
int MT::signal6 = 0;
int MT::signal7 = 0;
int MT::signal8 = 0;
int MT::signal9 = 0;
int MT::signal10 = 0;


MT* MT::MY = new MT();
MT* MT::instance()
{
    return MY;
}



std::string MT::m_v1;
void MT::show1(std::string v1)
{
    MT::signal1 = 1;
    MT::m_v1 = v1;
    printf("init display is begin\n");
    emit MY->mysignals1();
}

//case1
//1
SendInfoDisplay MT::case1_1;
HeaderDisplay MT::case1_2;
void MT::show2(SendInfoDisplay v1, HeaderDisplay v2)
{
    MT::signal2 = 1;
    MT::case1_1 = v1;
    MT::case1_2 = v2;
    printf("case1 1 is begin\n");
    emit MY->mysignals2();
}



//2
SendInfoDisplay MT::case1_3;
HeaderDisplay MT::case1_4;
VehicleIdentificationDisplay MT::case1_5;
void MT::show3(SendInfoDisplay v1, HeaderDisplay v2, VehicleIdentificationDisplay v3)
{
    MT::signal3 = 1;
    MT::case1_3 = v1;
    MT::case1_4 = v2;
    MT::case1_5 = v3;
    printf("case1 2 is begin\n");
    emit MY->mysignals3();
}

//3
SendInfoDisplay MT::case1_6;
HeaderDisplay MT::case1_7;
VehicleIdentificationDisplay MT::case1_8;
void MT::show4(SendInfoDisplay v1, HeaderDisplay v2, VehicleIdentificationDisplay v3)
{
    MT::signal4 = 1;
    MT::case1_6 = v1;
    MT::case1_7 = v2;
    MT::case1_8 = v3;
    printf("case1 3 is begin\n");
    emit MY->mysignals4();
}


//case2
//1
SendInfoDisplay MT::case2_3;
HeaderDisplay MT::case2_4;
EntityStatusDisplay MT::case2_5;
void MT::show5(SendInfoDisplay v1, HeaderDisplay v2, EntityStatusDisplay v3)
{
    MT::signal5 = 1;
    MT::case2_3 = v1;
    MT::case2_4 = v2;
    MT::case2_5 = v3;
    printf("case2 1 is begin\n");
    emit MY->mysignals5();
}


//case3
//1
SendInfoDisplay MT::case3_1;
HeaderDisplay MT::case3_2;
RoutingActivationRequestDisplay MT::case3_3;
void MT::show6(SendInfoDisplay v1, HeaderDisplay v2, RoutingActivationRequestDisplay v3)
{
    MT::signal6 = 1;
    MT::case3_1 = v1;
    MT::case3_2 = v2;
    MT::case3_3 = v3;
    printf("case3 1 is begin\n");
    emit MY->mysignals6();
}

//2
SendInfoDisplay MT::case3_4;
HeaderDisplay MT::case3_5;
RoutingActivationResponseDisplay MT::case3_6;
void MT::show7(SendInfoDisplay v1, HeaderDisplay v2, RoutingActivationResponseDisplay v3)
{
    MT::signal7 = 1;
    MT::case3_4 = v1;
    MT::case3_5 = v2;
    MT::case3_6 = v3;
    printf("case3 2 is begin\n");
    emit MY->mysignals7();
}


//case4
//1
std::string MT::case4_1;
void MT::show8(std::string v1)
{
    MT::signal8 = 1;
    MT::case4_1 = v1;
    printf("case4 1 is begin\n");
    emit MY->mysignals8();
}

//2
SendInfoDisplay MT::case4_2;
HeaderDisplay MT::case4_3;
DiagnosticMessageDisplay MT::case4_4;
void MT::show9(SendInfoDisplay v1, HeaderDisplay v2, DiagnosticMessageDisplay v3)
{
    MT::signal9 = 1;
    MT::case4_2 = v1;
    MT::case4_3 = v2;
    MT::case4_4 = v3;

    printf("case4 2 is begin\n");
    emit MY->mysignals9();
}

//3
SendInfoDisplay MT::case4_5;
HeaderDisplay MT::case4_6;
DiagnosticMessageAckDisplay MT::case4_7;
void MT::show10(SendInfoDisplay v1, HeaderDisplay v2, DiagnosticMessageAckDisplay v3)
{
    MT::signal10 = 1;
    MT::case4_5 = v1;
    MT::case4_6 = v2;
    MT::case4_7 = v3;
    printf("case4 3 is begin\n");
    emit MY->mysignals10();
}



SendInfoDisplay MT::case2_1;
HeaderDisplay MT::case2_2;
void MT::show22(SendInfoDisplay v1, HeaderDisplay v2)
{
    MT::signal22 = 1;
    MT::case2_1 = v1;
    MT::case2_2 = v2;
    printf("case1 1 is begin\n");
    emit MY->mysignals22();
}






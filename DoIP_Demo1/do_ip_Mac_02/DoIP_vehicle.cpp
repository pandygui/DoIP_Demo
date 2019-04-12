#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string>
#include <pthread.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "DoIP_pc.h"

int multicastSfd = 0;
int vehicleIdentificationSfd = 0;

int tcpListenSocket = 0;
int tcpAcceptSocket = 0 ;

char recvBuffer[DOIP_UDP_MESSAGE_MAX_SIZE] = { 0 };
char vehicleIp[DOIP_IPV6_ADDR_LEN] = { 0 };
char remotePcIp[DOIP_IPV6_ADDR_LEN] = { 0 };

bool createUdpSocket()
{
    printf("VehicleDoIP::createUdpSocket, create udp socket\n");
    vehicleIdentificationSfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (vehicleIdentificationSfd < 0) {
        printf("VehicleDoIP::createUdpSocket, create udp socket failed\n");
        return false;
    }

    printf("VehicleDoIP::createUdpSocket, create udp socket [%d] success\n", vehicleIdentificationSfd);
    return true;
}

bool createTcpSocket()
{
    tcpListenSocket = socket(AF_INET6, SOCK_STREAM, 0);
    if (tcpListenSocket < 0) {
        printf("VehicleDoIP::createTcpSocket, create socket failed \n");
        return false;
    }

    printf("VehicleDoIP::createTcpSocket, Create tcp socket [%d] success\n", tcpListenSocket);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    inet_pton(AF_INET6, vehicleIp, &saddr.sin6_addr);


    int on = 1;
    if (setsockopt(tcpListenSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        printf("VehicleDoIP::createTcpSocket, setsockopt SO_REUSEADDR failed!\n");
        close(tcpListenSocket);
        return false;
    }

    if (setsockopt(tcpListenSocket, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on))) {
        printf("VehicleDoIP::createTcpSocket, setsockopt SO_REUSEPORT failed\n");
        close(tcpListenSocket);
        return false;
    }

    if (bind(tcpListenSocket, (struct sockaddr *) &saddr, sizeof(saddr))) {
        printf("VehicleDoIP::createTcpSocket, bind failed!\n");
        close(tcpListenSocket);
        return false;
    }
    else {
        printf("VehicleDoIP::createTcpSocket, Bind tcp socket [%d] success\n", tcpListenSocket);
    }

    if (listen(tcpListenSocket, DOIP_MAX_TCP_SOCKET)) {
        printf("VehicleDoIP::createTcpSocket, listen failed!\n");
        close(tcpListenSocket);
        return false;
    }else {
        printf("VehicleDoIP::createTcpSocket, Listen tcp socket [%d] success\n", tcpListenSocket);
    }

    return true;
}

bool createMulticastSocket(const char* multicastIp, uint16_t multicastPort)
{
    printf("VehicleDoIP::createMulticastSocket, start, Multcast IP [%s], Multicast Port [%d]!\n", multicastIp, multicastPort);
    multicastSfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (multicastSfd < 0) {
        printf("VehicleDoIP::createMulticastSocket, create socket failed!\n");
        return false;
    }

    int on = 1;
    if (setsockopt(multicastSfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        printf("VehicleDoIP::createMulticastSocket, setsockopt SO_REUSEADDR failed!\n");
        close(multicastSfd);
        return false;
    }

    if (setsockopt(multicastSfd, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on))) {
        printf("VehicleDoIP::createMulticastSocket, setsockopt SO_REUSEPORT failed\n");
        close(multicastSfd);
        return false;
    }

    int hops = 255;
    if (setsockopt(multicastSfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops))) {
        printf("VehicleDoIP::createMulticastSocket, setsockopt IPV6_MULTICAST_HOPS failed!\n");
        close(multicastSfd);
        return false;
    }

    if (setsockopt(multicastSfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on, sizeof(on))) {
        printf("VehicleDoIP::createMulticastSocket, setsockopt IPV6_MULTICAST_LOOP failed!\n");
        close(multicastSfd);
        return false;
    }

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    saddr.sin6_addr = in6addr_any;

    if (bind(multicastSfd, (struct sockaddr *) &saddr, sizeof(saddr))) {
        printf("VehicleDoIP::createMulticastSocket, bind failed!\n");
        close(multicastSfd);
        return false;
    }

    struct sockaddr_in6 maddr;
    memset(&maddr, 0, sizeof(maddr));
    inet_pton(AF_INET6, DOIP_MULTICAST_IPV6_ADDR, &maddr.sin6_addr);

    struct ipv6_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    memcpy(&mreq.ipv6mr_multiaddr, &maddr.sin6_addr, sizeof(mreq.ipv6mr_multiaddr));
    mreq.ipv6mr_interface = 0;

    if (setsockopt(multicastSfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &mreq, sizeof(mreq))) {
        printf("VehicleDoIP::createMulticastSocket, setsockopt IPV6_JOIN_GROUP failed!\n");
        close(multicastSfd);
        return false;
    }

    printf("VehicleDoIP::createMulticastSocket, Create IPV6 multicast multicastSfd [%d] success\n", multicastSfd);
    return true;
}

void configureVehicleIdentificationResponse(struct VehicleIdentificationResponse& response)
{
    memset(&response, 0, sizeof(struct VehicleIdentificationResponse));

    response.header.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    response.header.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    response.header.payloadType = PAYLOAD_TYPE_RESPONSE_VEHICLE_IDENTIFICATION;
    response.header.payloadLength = 0x21;

    response.vin[0] = 0x01;
    response.vin[1] = 0x02;
    response.vin[2] = 0x03;
    response.vin[3] = 0x01;
    response.vin[4] = 0x02;
    response.vin[5] = 0x03;

    response.eid[0] = 0x04;
    response.eid[1] = 0x05;
    response.eid[2] = 0x06;
    response.eid[3] = 0x04;
    response.eid[4] = 0x05;
    response.eid[5] = 0x06;

    response.gid[0] = 0x07;
    response.gid[1] = 0x08;
    response.gid[2] = 0x09;
    response.gid[3] = 0x07;
    response.gid[4] = 0x08;
    response.gid[5] = 0x09;

    response.entityLA = 0x1222;
    response.furtherAction = 0x01;
    response.syncStatus = 0x01;
}


void configureEntityStatusResponse(struct EntityStatusResponse& response)
{
    memset(&response, 0, sizeof(struct EntityStatusResponse));

    response.header.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    response.header.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    response.header.payloadType = PAYLOAD_TYPE_RESPONSE_ENTITY_STATUS;
    response.header.payloadLength = 0x01;

    response.nodeType = 0x01;
    response.maxTcpSockets = DOIP_MAX_TCP_SOCKET;
    response.currentTcpSockets = 1;
    response.maxDataSize = DOIP_UDP_MESSAGE_MAX_SIZE;
}

void configureRoutingActivationResponse(struct RoutingActivationResponse& response)
{
    memset(&response, 0, sizeof(struct RoutingActivationResponse));

    response.header.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    response.header.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    response.header.payloadType = PAYLOAD_TYPE_REPONSE_ROUTING_ACTIVATION;
    response.header.payloadLength = 0x01;

    response.externalLA = 0x0E11;
    response.entityLA = 0x1222;
    response.responseCode = 0x10;
    response.reserved1 = 0;
    response.reserved2 = 0;
}

void configureDiagnosticMessagePositiveAck(struct DiagnosticMessagePositiveAck& response)
{
    memset(&response, 0, sizeof(struct DiagnosticMessagePositiveAck));

    response.header.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    response.header.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    response.header.payloadType = PAYLOAD_TYPE_POSITIVE_ACK_DIAGNOSTIC_MESSAGE;
    response.header.payloadLength = 0x07;

    response.entityLA = 0x1222;
    response.externalLA = 0x0E11;
    response.ackCode = 0;
    response.previousData = 0x8888;
}

bool responseVehicleIdentification(const char* dstIp, uint16_t dstPort) 
{
    printf("PCDoIP::responseVehicleIdentification, start, dst ip [%s], dstport [%d]\n", dstIp, dstPort);
    struct VehicleIdentificationResponse response;
    configureVehicleIdentificationResponse(response);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(dstPort);
    inet_pton(AF_INET6, dstIp, &saddr.sin6_addr);

    printf("PCDoIP::responseVehicleIdentification, payload_type [%d]\n", response.header.payloadType);

    int ret = sendto(vehicleIdentificationSfd, &response, sizeof(struct VehicleIdentificationResponse), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::responseVehicleIdentification, send message failed\n");
        return false;
    }

    return true;
}

bool responseEntityStatus(const char* dstIp, uint16_t dstPort)
{
    printf("PCDoIP::responseEntityStatus, start, dst ip [%s], dstport [%d]\n", dstIp, dstPort);
    struct EntityStatusResponse response;
    configureEntityStatusResponse(response);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(dstPort);
    inet_pton(AF_INET6, dstIp, &saddr.sin6_addr);

    int ret = sendto(vehicleIdentificationSfd, &response, sizeof(struct EntityStatusResponse), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::responseEntityStatus, send message failed\n");
        return false;
    }

    return true;
}

bool responseRoutingActivation(const char* dstIp, uint16_t dstPort)
{
    printf("PCDoIP::responseRoutingActivation, start, dst ip [%s], dstport [%d]\n", dstIp, dstPort);
    struct RoutingActivationResponse response;
    configureRoutingActivationResponse(response);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(dstPort);
    inet_pton(AF_INET6, dstIp, &saddr.sin6_addr);

    int ret = sendto(tcpAcceptSocket, &response, sizeof(struct RoutingActivationResponse), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::responseRoutingActivation, send message failed\n");
        return false;
    }
    return true;
}

bool responseDiagnosticMessagePositiveAck(const char* dstIp, uint16_t dstPort)
{
    printf("PCDoIP::responseDiagnosticMessagePositiveAck, start, dst ip [%s], dstport [%d]\n", dstIp, dstPort);
    struct DiagnosticMessagePositiveAck response;
    configureDiagnosticMessagePositiveAck(response);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(dstPort);
    inet_pton(AF_INET6, dstIp, &saddr.sin6_addr);

    int ret = sendto(tcpAcceptSocket, &response, sizeof(struct DiagnosticMessagePositiveAck), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::responseDiagnosticMessagePositiveAck, send message failed\n");
        return false;
    }
    return true;
}

bool parseReceiveMessage(const char* fromIp, uint16_t fromPort)
{
    printf("VehicleDoIP::parseReceiveMessage, start\n");
    struct DoIPHeader header;
    memcpy(&header, recvBuffer, sizeof(struct DoIPHeader));

    switch (header.payloadType) {
    case PAYLOAD_TYPE_REQUEST_VEHICLE_IDENTIFICATION:
    {
        printf("VehicleDoIP::parseReceiveMessage, Receive Vehicle Identification Request, Header Info: \nprotocolVersion = [0x%x], \ninverseProtocolVersion = [0x%x] \npayloadType = [0x%x], \npayloadLength = [0x%x]\n", 
            header.protocolVersion, header.inverseProtocolVersion, header.payloadType, header.payloadLength);
        if (!responseVehicleIdentification(fromIp, fromPort)) {
            return false;
        }
        return true;
    }
    case PAYLOAD_TYPE_REQUEST_ENTITY_STATUS:
    {
        printf("VehicleDoIP::parseReceiveMessage, Receive DoIP Entity Status Request, Header Info: \nprotocolVersion = [0x%x], \ninverseProtocolVersion = [0x%x] \npayloadType = [0x%x], \npayloadLength = [0x%x]\n", 
            header.protocolVersion, header.inverseProtocolVersion, header.payloadType, header.payloadLength);
        if (!responseEntityStatus(fromIp, fromPort)) {
            return false;
        }
        return true; 
    }
    case PAYLOAD_TYPE_REQUEST_ROUTING_ACTIVATION:
    {
        printf("VehicleDoIP::parseReceiveMessage, Receive Routing Activation Request, Header Info: \nprotocolVersion = [0x%x], \ninverseProtocolVersion = [0x%x] \npayloadType = [0x%x], \npayloadLength = [0x%x]\n", 
            header.protocolVersion, header.inverseProtocolVersion, header.payloadType, header.payloadLength);
        if (!responseRoutingActivation(fromIp, fromPort)) {
            return false;
        }
        return true; 
    }
    case PAYLOAD_TYPE_SEND_DIAGNOSTIC_MESSAGE:
    {
        printf("VehicleDoIP::parseReceiveMessage, Receive Diagnostic Message, Header Info: \nprotocolVersion = [0x%x], \ninverseProtocolVersion = [0x%x] \npayloadType = [0x%x], \npayloadLength = [0x%x]\n", 
            header.protocolVersion, header.inverseProtocolVersion, header.payloadType, header.payloadLength);
        if (!responseDiagnosticMessagePositiveAck(fromIp, fromPort)) {
            return false;
        }
        return true; 
    }
    default:
        printf("VehicleDoIP::parseReceiveMessage, payloadType failed\n");
        return false;
    }
}

void onReceiveMessage()
{
    printf("VehicleDoIP::onReceiveMessage, Receive thread start\n");

    bool isClientConnect = false;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(multicastSfd, &readfds);
        FD_SET(tcpListenSocket, &readfds);
        if (isClientConnect) {
            FD_SET(tcpAcceptSocket, &readfds);
        }

        int maxfds;
        if (multicastSfd > tcpListenSocket) {
            maxfds = multicastSfd + 1;
        }
        else {
            maxfds = tcpListenSocket + 1;
        }

        if (isClientConnect && (maxfds < tcpAcceptSocket + 1)) {
            maxfds = tcpAcceptSocket + 1;
        }

        struct timeval timeout;
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = DOIP_SOCKET_RECV_TIMEOUT;
        timeout.tv_usec = 0;
        int ret = select(maxfds, &readfds, NULL, NULL, &timeout);
        if (ret <= 0) {
            printf("VehicleDoIP::onReceiveMessage, select timeout\n");
            continue;
        }

        struct sockaddr_storage ssAddr;
        memset(&ssAddr, 0, sizeof(ssAddr));
        socklen_t socklen = sizeof(ssAddr);

        if (FD_ISSET(multicastSfd, &readfds)) {
            printf("VehicleDoIP::onReceiveMessage, Receive multicast message\n");
            ret = recvfrom(multicastSfd, &recvBuffer, sizeof(recvBuffer) - 1, 0, (struct sockaddr *)&ssAddr, &socklen);
            if (ret <= 0) {
                printf("VehicleDoIP::onReceiveMessage, Receive multicast message failed\n");
                continue;
            }

            char fromIp[DOIP_IPV6_ADDR_LEN] = { 0 };
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)&ssAddr)->sin6_addr, fromIp, sizeof(fromIp));
            uint32_t fromPort = ntohs(((struct sockaddr_in6*)&ssAddr)->sin6_port);

            printf("VehicleDoIP::onReceiveMessage, Receive multicast message from IP [%s], Port [%d]\n", fromIp, fromPort);
            parseReceiveMessage(fromIp, fromPort);
        }

        if (FD_ISSET(tcpListenSocket, &readfds)) {
            printf("VehicleDoIP::onReceiveMessage, Receive TCP listen message\n");
            struct sockaddr_in6 saddr;
            socklen_t saLen = sizeof(saddr);
            tcpAcceptSocket = accept(tcpListenSocket, (struct sockaddr*)&saddr, &saLen);
            isClientConnect = true;
            printf("VehicleDoIP::onReceiveMessage, Create TCP Accept socket [%d] success\n", tcpAcceptSocket);
        }

        if (isClientConnect && FD_ISSET(tcpAcceptSocket, &readfds)) {
            printf("VehicleDoIP::onReceiveMessage, Receive TCP message\n");
            ret = recvfrom(tcpAcceptSocket, &recvBuffer, sizeof(recvBuffer) - 1, 0, (struct sockaddr *)&ssAddr, &socklen);
            if (ret <= 0) {
                printf("VehicleDoIP::onReceiveMessage, Client socket closed\n");
                close(tcpAcceptSocket);
                isClientConnect = false;
                continue;
            }

            if (ret < DOIP_HEADER_SIZE) {
                printf("VehicleDoIP::onReceiveMessage, DoIP message is too short\n");
                continue;
            }

            char fromIp[DOIP_IPV6_ADDR_LEN] = { 0 };
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)&ssAddr)->sin6_addr, fromIp, sizeof(fromIp));
            uint32_t fromPort = ntohs(((struct sockaddr_in6*)&ssAddr)->sin6_port);

            printf("VehicleDoIP::onReceiveMessage, Receive TCP message from IP [%s], Port [%d]\n", fromIp, fromPort);
            parseReceiveMessage(fromIp, fromPort);
        }

        continue;
    }
}

bool readIfConfigFile()
{
    printf("VehicleDoIP::readIpConfigFile, start\n");
    FILE* fp = fopen(CONFIG_FILE_PATH, "r");
    if (NULL == fp) {
        printf("VehicleDoIP::readIpConfigFile, fopen failed\n");
        return false;
    }

    char lineBuf[128] = { 0 };
    char *tmp = NULL;
    int configNum = 0;
    while (NULL != fgets(lineBuf, sizeof(lineBuf) -1, fp)) {
        tmp = lineBuf;
        while (*tmp && isspace(*tmp)) {
            ++tmp;
        }
        
        if (strncmp(tmp, PC_INTERFACE_IP, strlen(PC_INTERFACE_IP)) == 0) {
            tmp = tmp + strlen(PC_INTERFACE_IP) + strlen("=");
            sscanf(tmp, "%64s", remotePcIp);
            printf("VehicleDoIP::readIpConfigFile, Remote PC IP Addr [%s]\n", remotePcIp);
            configNum++;
            if (2 == configNum) {
                return true;
            }
            continue;
        }
        else if (strncmp(tmp, VEHICLE_INTERFACE_IP, strlen(VEHICLE_INTERFACE_IP)) == 0) {
            tmp = tmp + strlen(VEHICLE_INTERFACE_IP) + strlen("=");
            sscanf(tmp, "%64s", vehicleIp);
            printf("VehicleDoIP::readIpConfigFile, Vehicle IP Addr [%s]\n", vehicleIp);
            configNum++;
            if (2 == configNum) {
                return true;
            }
            continue;
        }
        else {
            continue;
        }
    }

    fclose(fp);
    return false;
}

bool notifyVehicleIdentification(const char* dstIp, uint16_t dstPort)
{
    printf("PCDoIP::notifyVehicleIdentification, Start\n");
    for (int i = 0; i < ANNOUNCEMENT_VEHICLE_IDENTIFICATION_TIMES; i++) {
        usleep(ANNOUNCEMENT_VEHICLE_IDENTIFICATION_INTERVAL);
        printf("PCDoIP::notifyVehicleIdentification, Send notify [%d] times\n", i + 1);
        if (!responseVehicleIdentification(DOIP_MULTICAST_IPV6_ADDR, DOIP_PORT)) {
            return false;
        }
    }

    return true;
}

int main()
{
    printf("VehicleDoIP::main, start\n");
    if (!readIfConfigFile()) {
        printf("VehicleDoIP::main, read config file failed\n");
        return -1;
    }

    if (!createUdpSocket()) {
        printf("VehicleDoIP::main, create udp socket failed\n");
        return -1;
    }

    if (!createTcpSocket()) {
        printf("VehicleDoIP::main, create tcp socket failed\n");
        return -1;
    }

    if (!createMulticastSocket(DOIP_MULTICAST_IPV6_ADDR, DOIP_PORT)) {
        printf("VehicleDoIP::main, createMulticastSocket failed\n");
        return -1;
    }

    if (!notifyVehicleIdentification(DOIP_MULTICAST_IPV6_ADDR, DOIP_PORT)) {
        printf("VehicleDoIP::main, notify vehicle identification failed\n");
        return -1;
    }

    onReceiveMessage();
    return 0;
}








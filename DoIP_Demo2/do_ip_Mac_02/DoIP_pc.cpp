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
#include <net/if.h>
#include <pthread.h>
#include <errno.h>

#include "DoIP_pc.h"

// #include "mytest.h"

int sfdVehicleIdentification = 0;
int multicastSfd = 0;
int tcpClientSfd = 0;

bool isRoutingActivation = false;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char pcIp[DOIP_IPV6_ADDR_LEN] = { 0 };
char remoteVehicleIp[DOIP_IPV6_ADDR_LEN] = { 0 };
char recvBuffer[DOIP_UDP_MESSAGE_MAX_SIZE] = { 0 };

bool getRoutingActivation()
{
    pthread_mutex_lock(&mutex);
    bool ret = isRoutingActivation;
    pthread_mutex_unlock(&mutex);
    return ret;
}

void setRoutingActivation(bool status)
{
    pthread_mutex_lock(&mutex);
    isRoutingActivation = status;
    pthread_mutex_unlock(&mutex);
}

void convertHeaderDisplay(const struct DoIPHeader& header, struct HeaderDisplay& headerDisplay)
{
    char protocolVersion[UINT8_HEX + OTHER_HEX] = { 0 };
    char inverseProtocolVersion[UINT8_HEX + OTHER_HEX] = { 0 };
    char payloadType[UINT16_HEX + OTHER_HEX] = { 0 };
    char payloadLength[UINT32_HEX + OTHER_HEX] = { 0 };

    snprintf(protocolVersion, UINT8_HEX + OTHER_HEX - 1, "0x%x", header.protocolVersion);
    snprintf(inverseProtocolVersion, UINT8_HEX + OTHER_HEX - 1, "0x%x", header.inverseProtocolVersion);
    snprintf(payloadType, UINT16_HEX + OTHER_HEX - 1, "0x%x", header.payloadType);
    snprintf(payloadLength, UINT32_HEX + OTHER_HEX - 1, "0x%x", header.payloadLength);

    headerDisplay.protocolVersion = protocolVersion;
    headerDisplay.inverseProtocolVersion = inverseProtocolVersion;
    headerDisplay.payloadType = payloadType;
    headerDisplay.payloadLength = payloadLength;
}

void convertSendDisplayPort(const uint16_t srcPort, const uint16_t dstPort, struct SendInfoDisplay& sendDisplay)
{
    char srcPortStr[6] = { 0 };
    char dstPortStr[6] = { 0 };
    snprintf(srcPortStr, 5, "%d", srcPort);
    snprintf(dstPortStr, 5, "%d", dstPort);

    sendDisplay.srcPort = srcPortStr;
    sendDisplay.dstPort = dstPortStr;
}

bool createUdpSocket()
{
    printf("PCDoIP::createUdpSocket, create udp socket start\n");
    sfdVehicleIdentification = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sfdVehicleIdentification < 0) {
        printf("PCDoIP::createUdpSocket, create udp socket failed\n");
        return false;
    }
    
    return true;
}

bool createTcpSocket()
{
    tcpClientSfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (tcpClientSfd < 0) {
        printf("VehicleDoIP::createTcpSocket, create socket failed \n");
        return false;
    }

    return true;
}

bool createMulticastSocket(const char* multicastIp, uint16_t multicastPort)
{
    printf("PCDoIP::createMulticastSocket, start, Multcast IP [%s], Multicast Port [%d]!\n", multicastIp, multicastPort);
    multicastSfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (multicastSfd < 0) {
        printf("PCDoIP::createMulticastSocket, create socket failed!\n");
        return false;
    }

    int on = 1;
    if (setsockopt(multicastSfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        printf("PCDoIP::createMulticastSocket, setsockopt SO_REUSEADDR failed!\n");
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
        printf("PCDoIP::createMulticastSocket, setsockopt IPV6_MULTICAST_HOPS failed!\n");
        close(multicastSfd);
        return false;
    }

    if (setsockopt(multicastSfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on, sizeof(on))) {
        printf("PCDoIP::createMulticastSocket, setsockopt IPV6_MULTICAST_LOOP failed!\n");
        close(multicastSfd);
        return false;
    }

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    saddr.sin6_addr = in6addr_any;

    if (bind(multicastSfd, (struct sockaddr *) &saddr, sizeof(saddr))) {
        printf("PCDoIP::createMulticastSocket, bind failed!\n");
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
        printf("PCDoIP::createMulticastSocket, setsockopt IPV6_JOIN_GROUP failed!\n");
        close(multicastSfd);
        return false;
    }

    printf("PCDoIP::createMulticastSocket, Create IPV6 multicast multicastSfd [%d] success\n", multicastSfd);
    return true;
}

bool getLocalIpPort(int sfd, char* localIp, uint16_t& localPort)
{
    struct sockaddr localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localAddrLen = sizeof(struct sockaddr);

    if (0 != getsockname(sfd, &localAddr, &localAddrLen)){
        printf("PCDoIP::getLocalIpPort, getsockname failed\n");
        return false;
    }

    if (NULL == inet_ntop(AF_INET6, &((struct sockaddr_in6*)&localAddr)->sin6_addr, localIp, DOIP_IPV6_ADDR_LEN)) {
        printf("PCDoIP::getLocalIpPort, inet_ntop failed\n");
        return false;
    }

    localPort = ntohs(((struct sockaddr_in6*)&localAddr)->sin6_port);
    printf("PCDoIP::getLocalIpPort, Local IP [%s], Local Port[%d]\n", localIp, localPort);
    return true;
}

void configureVehicleIdentificationRequest(struct DoIPHeader& request)
{
    request.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    request.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    request.payloadType = PAYLOAD_TYPE_REQUEST_VEHICLE_IDENTIFICATION;
    request.payloadLength = 0;
}

void configureEntityStatusRequest(struct DoIPHeader& request)
{
    request.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    request.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    request.payloadType = PAYLOAD_TYPE_REQUEST_ENTITY_STATUS;
    request.payloadLength = 0;
}

void configureRoutingActivationRequest(struct RoutingActivationRequest& request)
{
    request.header.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    request.header.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    request.header.payloadType = PAYLOAD_TYPE_REQUEST_ROUTING_ACTIVATION;
    request.header.payloadLength = 0x0B;

    request.externalLA = 0x0E11;
    request.activationType = 0x00;
    request.reserved1= 0;
    request.reserved2= 0;
}

void configureDiagnosticMessageSend(struct DiagnosticMessageRequest& request)
{
    request.header.protocolVersion = DOIP_HEADER_PROTOCOL_VERSION;
    request.header.inverseProtocolVersion = DOIP_HEADER_INVERSE_PROTOCOL_VERSION;
    request.header.payloadType = PAYLOAD_TYPE_SEND_DIAGNOSTIC_MESSAGE;
    request.header.payloadLength = 0x06;

    request.externalLA = 0x0E11;
    request.entityLA = 0x1222;
    request.userData= 0x8888;
}

bool parseReceiveMessage(int sfd, const char* fromIp, uint16_t fromPort)
{
    struct DoIPHeader header;
    memcpy(&header, recvBuffer, sizeof(struct DoIPHeader));

    printf("PCDoIP::parseReceiveMessage, start payload type = [%x]\n", header.payloadType);
    switch (header.payloadType) {
    case PAYLOAD_TYPE_RESPONSE_VEHICLE_IDENTIFICATION:
    {
        struct VehicleIdentificationResponse response;
        memcpy(&response, recvBuffer, sizeof(struct VehicleIdentificationResponse));

        char localIP[DOIP_IPV6_ADDR_LEN];
        uint16_t localPort;
        if (!getLocalIpPort(sfd, localIP, localPort)) {
            printf("PCDoIP::parseReceiveMessage, getLocalIpPort failed\n");
            return false;
        }
        else {
            printf("\n****************************Resonse Vehicle Identification Start*********************************\n");
            printf("Local IP:Local Port  [%s]:[%d]  <--------- Remote IP:Remote Port [%s]:[%d]\n",  
            pcIp, localPort, remoteVehicleIp, fromPort);
        }

        struct SendInfoDisplay sendDisplay;
        convertSendDisplayPort(fromPort, localPort, sendDisplay);
        sendDisplay.srcIp = remoteVehicleIp;
        sendDisplay.dstIp = pcIp;

        struct HeaderDisplay hDisplay;
        convertHeaderDisplay(header, hDisplay);

        char vin[UINT8_HEX*17 + OTHER_HEX] = { 0 };
        char entityLA[UINT16_HEX + OTHER_HEX] = { 0 };
        char eid[UINT8_HEX*6 + OTHER_HEX] = { 0 };
        char gid[UINT8_HEX*6 + OTHER_HEX] = { 0 };
        char furtherAction[UINT8_HEX + OTHER_HEX] = { 0 };
        char syncStatus[UINT8_HEX + OTHER_HEX] = { 0 };

        snprintf(vin, UINT8_HEX*17 + OTHER_HEX - 1, "0x%x%x%x%x%x%x", response.vin[0], response.vin[1], response.vin[2], response.vin[3], response.vin[4], response.vin[5]);
        snprintf(entityLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", response.entityLA);
        snprintf(eid, UINT8_HEX*6 + OTHER_HEX - 1, "0x%x%x%x%x%x%x", response.eid[0], response.eid[1], response.eid[2], response.eid[3], response.eid[4], response.eid[5]);
        snprintf(gid, UINT8_HEX*6 + OTHER_HEX - 1, "0x%x%x%x%x%x%x", response.gid[0], response.gid[1], response.gid[2], response.gid[3], response.gid[4], response.gid[5]);
        snprintf(furtherAction, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.furtherAction);
        snprintf(syncStatus, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.syncStatus);
        
        struct VehicleIdentificationDisplay payloadDisplay;
        payloadDisplay.vin = vin;

        payloadDisplay.entityLA = entityLA;
        payloadDisplay.eid = eid;
        payloadDisplay.gid = gid;
        payloadDisplay.furtherAction = furtherAction;
        payloadDisplay.syncStatus = syncStatus;

        printf("\nHeader Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
            hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());

        printf("\nPayload Info List: \nVIN = [%s], \nEID = [%s], \nGID = [%s], \nLogical Addr = [%s], \nFurther Action = [%s], \nVIN/GID Sync Status = [%s]\n",
            payloadDisplay.vin.c_str(), payloadDisplay.eid.c_str(), payloadDisplay.gid.c_str(), payloadDisplay.entityLA.c_str(), payloadDisplay.furtherAction.c_str(), payloadDisplay.syncStatus.c_str());
        printf("****************************Resonse Vehicle Identification End*********************************\n\n");


        if (sfdVehicleIdentification == sfd) {
            responseVehicleIdentificationDisplay(sendDisplay, hDisplay, payloadDisplay);
        }
        else if (multicastSfd == sfd) {
            announcementVehicleIdentificationDisplay(sendDisplay, hDisplay, payloadDisplay);
        }
        else {
            printf("PCDoIP::parseReceiveMessage, sfd parameter failed\n");
            return false;
        }
        break;
    }
    case PAYLOAD_TYPE_RESPONSE_ENTITY_STATUS:
    {
        struct EntityStatusResponse response;
        memcpy(&response, recvBuffer, sizeof(struct EntityStatusResponse));

        char localIP[DOIP_IPV6_ADDR_LEN];
        uint16_t localPort;
        if (!getLocalIpPort(sfd, localIP, localPort)) {
            printf("PCDoIP::parseReceiveMessage, getLocalIpPort failed\n");
            return false;
        }
        else {
            printf("\n****************************Resonse Entity Status Start*********************************\n");
            printf("Local IP:Local Port  [%s]:[%d]  <--------- Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, remoteVehicleIp, fromPort);
        }

        struct SendInfoDisplay sendDisplay;
        convertSendDisplayPort(fromPort, localPort, sendDisplay);
        sendDisplay.srcIp = remoteVehicleIp;
        sendDisplay.dstIp = pcIp;

        struct HeaderDisplay hDisplay;
        convertHeaderDisplay(header, hDisplay);

        char nodeType[UINT8_HEX + OTHER_HEX] = { 0 };
        char maxTcpSockets[UINT8_HEX + OTHER_HEX] = { 0 };
        char currentTcpSockets[UINT8_HEX + OTHER_HEX] = { 0 };
        char maxDataSize[UINT32_HEX + OTHER_HEX] = { 0 };

        snprintf(nodeType, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.nodeType);
        snprintf(maxTcpSockets, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.maxTcpSockets);
        snprintf(currentTcpSockets, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.currentTcpSockets);
        snprintf(maxDataSize, UINT32_HEX + OTHER_HEX, "0x%x", response.maxDataSize);

        struct EntityStatusDisplay payloadDisplay;
        payloadDisplay.nodeType = nodeType;
        payloadDisplay.maxTcpSockets = maxTcpSockets;
        payloadDisplay.currentTcpSockets = currentTcpSockets;
        payloadDisplay.maxDataSize = maxDataSize;

        printf("\nHeader Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
            hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());

        printf("\nPayload Info List: \nNode Type = [%s] \nMax cocurrent TCP_DATA sockets = [%s] \nCurrent open TCP_DATA sockets = [%s] \nMax Data Size = [%s]\n", 
            payloadDisplay.nodeType.c_str(), payloadDisplay.maxTcpSockets.c_str(), payloadDisplay.currentTcpSockets.c_str(), payloadDisplay.maxDataSize.c_str());
        printf("****************************Resonse Entity Status End*********************************\n\n");

        responseDoipEntityStatusDisplay(sendDisplay, hDisplay, payloadDisplay);
        break;
    }
    case PAYLOAD_TYPE_REPONSE_ROUTING_ACTIVATION:
    {
        struct RoutingActivationResponse response;
        memcpy(&response, recvBuffer, sizeof(struct RoutingActivationResponse));

        char localIP[DOIP_IPV6_ADDR_LEN];
        uint16_t localPort;
        if (!getLocalIpPort(sfd, localIP, localPort)) {
            printf("PCDoIP::parseReceiveMessage, getLocalIpPort failed\n");
            return false;
        }
        else {
            printf("\n****************************Resonse Routing Activation Start*********************************\n");
            printf("Local IP:Local Port  [%s]:[%d]  <--------- Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, remoteVehicleIp, fromPort);
        }

        struct SendInfoDisplay sendDisplay;
        convertSendDisplayPort(fromPort, localPort, sendDisplay);
        sendDisplay.srcIp = remoteVehicleIp;
        sendDisplay.dstIp = pcIp;

        struct HeaderDisplay hDisplay;
        convertHeaderDisplay(header, hDisplay);

        char externalLA[UINT16_HEX + OTHER_HEX] = { 0 };
        char entityLA[UINT16_HEX + OTHER_HEX] = { 0 };
        char responseCode[UINT8_HEX + OTHER_HEX] = { 0 };

        snprintf(externalLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", response.externalLA);
        snprintf(entityLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", response.entityLA);
        snprintf(responseCode, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.responseCode);

        struct RoutingActivationResponseDisplay payloadDisplay;
        payloadDisplay.externalLA = externalLA;
        payloadDisplay.entityLA = entityLA;
        payloadDisplay.responseCode = responseCode;

        printf("\nHeader Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
            hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());

        printf("\nPayload Info List: \nExternal Logical Addr = [%s] \nEntity Logical Addr = [%s] \nResponse Code = [%s]\n", 
            payloadDisplay.externalLA.c_str(), payloadDisplay.entityLA.c_str(), payloadDisplay.responseCode.c_str());
        printf("****************************Resonse Routing Activation End*********************************\n\n");

        responseRoutingActivationoDisplay(sendDisplay, hDisplay, payloadDisplay);
        break;
    }
    case PAYLOAD_TYPE_POSITIVE_ACK_DIAGNOSTIC_MESSAGE:
    {
        struct DiagnosticMessagePositiveAck response;
        memcpy(&response, recvBuffer, sizeof(struct DiagnosticMessagePositiveAck));

        char localIP[DOIP_IPV6_ADDR_LEN];
        uint16_t localPort;
        if (!getLocalIpPort(sfd, localIP, localPort)) {
            printf("PCDoIP::parseReceiveMessage, getLocalIpPort failed\n");
            return false;
        }
        else {
            printf("\n****************************Resonse Diagnostic Positive Ack Start*********************************\n");
            printf("Local IP:Local Port  [%s]:[%d]  <--------- Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, remoteVehicleIp, fromPort);
        }

        struct SendInfoDisplay sendDisplay;
        convertSendDisplayPort(fromPort, localPort, sendDisplay);
        sendDisplay.srcIp = remoteVehicleIp;
        sendDisplay.dstIp = pcIp;

        struct HeaderDisplay hDisplay;
        convertHeaderDisplay(header, hDisplay);

        char entityLA[UINT16_HEX + OTHER_HEX] = { 0 };
        char externalLA[UINT16_HEX + OTHER_HEX] = { 0 };
        char ackCode[UINT8_HEX + OTHER_HEX] = { 0 };
        char previousData[UINT16_HEX + OTHER_HEX] = { 0 };

        snprintf(entityLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", response.entityLA);
        snprintf(externalLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", response.externalLA);
        snprintf(ackCode, UINT8_HEX + OTHER_HEX - 1, "0x%x", response.ackCode);
        snprintf(previousData, UINT16_HEX + OTHER_HEX - 1, "0x%x", response.previousData);

        struct DiagnosticMessageAckDisplay payloadDisplay;
        payloadDisplay.entityLA = entityLA;
        payloadDisplay.externalLA = externalLA;
        payloadDisplay.ackCode = ackCode;
        payloadDisplay.previousData = previousData;

        printf("\nHeader Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
            hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());

        printf("\nPayload Info List: \nEntity Logical Addr = [%s] \nExternal Logical Addr = [%s]  \nAck Code = [%s] \nPrevious Data = [%s]\n", 
            payloadDisplay.entityLA.c_str(), payloadDisplay.externalLA.c_str(), payloadDisplay.ackCode.c_str(), payloadDisplay.previousData.c_str());
        printf("****************************Resonse Diagnostic Positive Ack End*********************************\n\n");

        responseDiagnosticMessageAckDisplay(sendDisplay, hDisplay, payloadDisplay);
        break;
    }
    default:
        printf("VehicleDoIP::parseReceiveMessage, payloadType failed\n");
        return false;
    }

    return true;
}

void* onReceiveMessage(void* arg)
{
    printf("PCDoIP::onReceiveMessage, Receive thread start\n");
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sfdVehicleIdentification, &readfds);
        FD_SET(multicastSfd, &readfds);

        if (getRoutingActivation()) {
            FD_SET(tcpClientSfd, &readfds);
        }
        
        int maxfds;
        if (multicastSfd > sfdVehicleIdentification) {
            maxfds = multicastSfd + 1;
        }
        else {
            maxfds = sfdVehicleIdentification + 1;
        }

        if (getRoutingActivation() && (maxfds < tcpClientSfd + 1)) {
            maxfds = tcpClientSfd + 1;
        }

        struct timeval timeout;
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = DOIP_SOCKET_RECV_TIMEOUT;
        timeout.tv_usec = 0;
        int ret = select(maxfds, &readfds, NULL, NULL, &timeout);
        if (ret <= 0) {
            printf("PCDoIP::onReceiveMessage, select timeout\n");
            continue;
        }

        struct sockaddr_storage ssAddr;
        socklen_t socklen = sizeof(ssAddr);

        if (FD_ISSET(sfdVehicleIdentification, &readfds)) {
            printf("PCDoIP::onReceiveMessage, Receive unicast message\n");
            ret = recvfrom(sfdVehicleIdentification, &recvBuffer, sizeof(recvBuffer) - 1, 0, (struct sockaddr *)&ssAddr, &socklen);
            if (ret > 0) { 
                char fromIp[DOIP_IPV6_ADDR_LEN];
                inet_ntop(AF_INET6, &((struct sockaddr_in6*)&ssAddr)->sin6_addr, fromIp, sizeof(fromIp));
                uint32_t fromPort = ntohs(((struct sockaddr_in6*)&ssAddr)->sin6_port);

                printf("PCDoIP::onReceiveMessage, receive unicast message from IP [%s], Port [%d]\n", fromIp, fromPort);
                parseReceiveMessage(sfdVehicleIdentification, fromIp, fromPort);
            }
        }

        if (FD_ISSET(multicastSfd, &readfds)) {
            printf("PCDoIP::onReceiveMessage, Receive multicast message\n");
            ret = recvfrom(multicastSfd, &recvBuffer, sizeof(recvBuffer) - 1, 0, (struct sockaddr *)&ssAddr, &socklen);
            if (ret > 0) {
                char fromIp[DOIP_IPV6_ADDR_LEN];
                inet_ntop(AF_INET6, &((struct sockaddr_in6*)&ssAddr)->sin6_addr, fromIp, sizeof(fromIp));
                uint32_t fromPort = ntohs(((struct sockaddr_in6*)&ssAddr)->sin6_port);

                printf("PCDoIP::onReceiveMessage, receive multicast message from IP [%s], Port [%d]\n", fromIp, fromPort);
                parseReceiveMessage(multicastSfd, fromIp, fromPort); 
            }
        }

        if (getRoutingActivation() && FD_ISSET(tcpClientSfd, &readfds)) {
            printf("PCDoIP::onReceiveMessage, Receive TCP Response message\n");
            ret = recvfrom(tcpClientSfd, &recvBuffer, sizeof(recvBuffer) - 1, 0, (struct sockaddr *)&ssAddr, &socklen);
            if (ret <= 0) {
                printf("PCDoIP::onReceiveMessage, Server socket closed\n");
                setRoutingActivation(false);
                continue;
            }

            if (ret < DOIP_HEADER_SIZE) {
                printf("PCDoIP::onReceiveMessage, DoIP message is too short\n");
                continue;
            }

            char fromIp[DOIP_IPV6_ADDR_LEN];
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)&ssAddr)->sin6_addr, fromIp, sizeof(fromIp));
            uint32_t fromPort = ntohs(((struct sockaddr_in6*)&ssAddr)->sin6_port);

            printf("PCDoIP::onReceiveMessage, receive multicast message from IP [%s], Port [%d]\n", fromIp, fromPort);
            parseReceiveMessage(tcpClientSfd, fromIp, fromPort); 

        }

        continue;
    }
}

bool requestVehicleIdentification()
{
    printf("PCDoIP::requestVehicleIdentification, start\n");
    struct DoIPHeader header;
    configureVehicleIdentificationRequest(header);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    inet_pton(AF_INET6, DOIP_MULTICAST_IPV6_ADDR, &saddr.sin6_addr);

    int ret = sendto(sfdVehicleIdentification, &header, sizeof(struct DoIPHeader), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::requestVehicleIdentification, send message failed\n");
        return false;
    }

    char localIP[DOIP_IPV6_ADDR_LEN];
    uint16_t localPort;
    if (!getLocalIpPort(sfdVehicleIdentification, localIP, localPort)) {
        printf("PCDoIP::requestVehicleIdentification, getLocalIpPort failed\n");
        return false;
    }
    else {
        printf("\n****************************Request Vehicle Identification Start*********************************\n");
        printf("Local IP:Local Port  [%s]:[%d]  ---------> Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, DOIP_MULTICAST_IPV6_ADDR, DOIP_PORT);
    }

    struct SendInfoDisplay sendDisplay;
    convertSendDisplayPort(localPort, DOIP_PORT, sendDisplay);
    sendDisplay.srcIp = pcIp;
    sendDisplay.dstIp = DOIP_MULTICAST_IPV6_ADDR;

    struct HeaderDisplay hDisplay;
    convertHeaderDisplay(header, hDisplay);

    printf("\nHead Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
        hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());
    printf("****************************Request Vehicle Identification End*********************************\n\n");
    requestDisplay(sendDisplay, hDisplay);
    return true;
}

bool requestEntityStatus()
{
    printf("PCDoIP::requestEntityStatus, start\n");
    struct DoIPHeader header;
    configureEntityStatusRequest(header);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    inet_pton(AF_INET6, DOIP_MULTICAST_IPV6_ADDR, &saddr.sin6_addr);

    int ret = sendto(sfdVehicleIdentification, &header, sizeof(struct DoIPHeader), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::requestEntityStatus, send message failed\n");
        return false;
    }

    char localIP[DOIP_IPV6_ADDR_LEN];
    uint16_t localPort;
    if (!getLocalIpPort(sfdVehicleIdentification, localIP, localPort)) {
        printf("PCDoIP::requestEntityStatus, getLocalIpPort failed\n");
        return false;
    }
    else {
        printf("\n****************************Request Entity Status Start*********************************\n");
        printf("Local IP:Local Port  [%s]:[%d]  ---------> Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, remoteVehicleIp, DOIP_PORT);
    } 

    struct SendInfoDisplay sendDisplay;
    convertSendDisplayPort(localPort, DOIP_PORT, sendDisplay);
    sendDisplay.srcIp = pcIp;
    sendDisplay.dstIp = DOIP_MULTICAST_IPV6_ADDR;

    struct HeaderDisplay hDisplay;
    convertHeaderDisplay(header, hDisplay);

    printf("\nHead Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
        hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());
    printf("****************************Request Entity Status End*********************************\n\n");
    requestDisplay(sendDisplay, hDisplay);
    return true;
}

bool requestRoutingActivation()
{
    printf("PCDoIP::requestRoutingActivation, start\n");
    struct RoutingActivationRequest request;
    configureRoutingActivationRequest(request);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    inet_pton(AF_INET6, remoteVehicleIp, &saddr.sin6_addr);

    if (!getRoutingActivation()) {
        int ret = connect(tcpClientSfd, (struct sockaddr*)&saddr, sizeof(saddr));
        
        if (ret) {
            printf("PCDoIP::requestRoutingActivation, Connect server failed, ret [%d], error [%d]\n", ret, errno);
            return false;     
        }
        else {
            printf("PCDoIP::requestRoutingActivation, Connect server [%s]:[%d] success\n", remoteVehicleIp, DOIP_PORT);
            setRoutingActivation(true);            
        }
    }

    int ret = sendto(tcpClientSfd, &request, sizeof(struct RoutingActivationRequest), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::requestRoutingActivation, send message failed\n");
        return false;
    }

    char localIP[DOIP_IPV6_ADDR_LEN];
    uint16_t localPort;
    if (!getLocalIpPort(tcpClientSfd, localIP, localPort)) {
        printf("PCDoIP::requestRoutingActivation, getLocalIpPort failed\n");
        return false;
    }
    else {
        printf("\n****************************Request Routing Activation Start*********************************\n");

        printf("Local IP:Local Port  [%s]:[%d]  ---------> Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, remoteVehicleIp, DOIP_PORT);
    } 

    struct SendInfoDisplay sendDisplay;
    convertSendDisplayPort(localPort, DOIP_PORT, sendDisplay);
    sendDisplay.srcIp = pcIp;
    sendDisplay.dstIp = remoteVehicleIp;

    struct HeaderDisplay hDisplay;
    convertHeaderDisplay(request.header, hDisplay);

    char externalLA[UINT16_HEX + OTHER_HEX] = { 0 };
    char activationType[UINT8_HEX + OTHER_HEX] = { 0 };

    snprintf(externalLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", request.externalLA);
    snprintf(activationType, UINT8_HEX + OTHER_HEX - 1, "0x%x", request.activationType);

    struct RoutingActivationRequestDisplay payloadDisplay;
    payloadDisplay.externalLA =  externalLA;
    payloadDisplay.activationType = activationType;

    printf("\nHead Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
        hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());

    printf("\nPayload Info List: \nexternalLA = [%s], \nactivationType = [%s] \n", 
        payloadDisplay.externalLA.c_str(), payloadDisplay.activationType.c_str());

    printf("******************************Request Routing Activation End*********************************\n\n");

    reqeustRoutingActivationoDisplay(sendDisplay, hDisplay, payloadDisplay);
    return true;
}

bool sendDiagnosticMessage()
{
    printf("PCDoIP::sendDiagnosticMessage, start\n");
    if (!getRoutingActivation()) {
        printf("PCDoIP::sendDiagnosticMessage, routing is not activation\n");
        needRoutingActivationDisplay(NEED_ROUTING_ACTIVATION);
        return false;
    }

    struct DiagnosticMessageRequest request;
    configureDiagnosticMessageSend(request);

    struct sockaddr_in6 saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in6));
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(DOIP_PORT);
    inet_pton(AF_INET6, remoteVehicleIp, &saddr.sin6_addr);

    int ret = sendto(tcpClientSfd, &request, sizeof(struct DiagnosticMessageRequest), 0, (struct sockaddr*)&saddr, sizeof(saddr));
    if (-1 == ret) {
        printf("PCDoIP::sendDiagnosticMessage, send message failed\n");
        return false;
    }

    char localIP[DOIP_IPV6_ADDR_LEN];
    uint16_t localPort;
    if (!getLocalIpPort(sfdVehicleIdentification, localIP, localPort)) {
        printf("PCDoIP::sendDiagnosticMessage, getLocalIpPort failed\n");
        return false;
    }
    else {
        printf("\n****************************Send Diagnostic Message Start*********************************\n");
        printf("Local IP:Local Port  [%s]:[%d]  ---------> Remote IP:Remote Port [%s]:[%d]\n",   
            pcIp, localPort, remoteVehicleIp, DOIP_PORT);

        printf("\nHead Info List: \nprotocolVersion = [0x%x], \ninverseProtocolVersion = [0x%x] \npayloadType = [0x%x], \npayloadLength = [%x]\n", 
            request.header.protocolVersion, request.header.inverseProtocolVersion, request.header.payloadType, request.header.payloadLength);
        printf("****************************Send Diagnostic Message End*********************************\n\n");
    } 

    struct SendInfoDisplay sendDisplay;
    convertSendDisplayPort(localPort, DOIP_PORT, sendDisplay);
    sendDisplay.srcIp = pcIp;
    sendDisplay.dstIp = remoteVehicleIp;

    struct HeaderDisplay hDisplay;
    convertHeaderDisplay(request.header, hDisplay);

    char externalLA[UINT16_HEX + OTHER_HEX] = { 0 };
    char entityLA[UINT16_HEX + OTHER_HEX] = { 0 };
    char userData[UINT16_HEX + OTHER_HEX] = { 0 };

    snprintf(externalLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", request.externalLA);
    snprintf(entityLA, UINT16_HEX + OTHER_HEX - 1, "0x%x", request.entityLA);
    snprintf(userData, UINT16_HEX + OTHER_HEX - 1, "0x%x", request.userData);

    struct DiagnosticMessageDisplay payloadDisplay;
    payloadDisplay.externalLA = externalLA;
    payloadDisplay.entityLA = entityLA;
    payloadDisplay.userData = userData;

    printf("\nHead Info List: \nprotocolVersion = [%s], \ninverseProtocolVersion = [%s] \npayloadType = [%s], \npayloadLength = [%s]\n", 
        hDisplay.protocolVersion.c_str(), hDisplay.inverseProtocolVersion.c_str(), hDisplay.payloadType.c_str(), hDisplay.payloadLength.c_str());

    printf("\nPayload Info List: \nexternalLA = [%s], \nentityLA = [%s] , \nuserData = [%s]\n", 
        payloadDisplay.externalLA.c_str(), payloadDisplay.entityLA.c_str(), payloadDisplay.userData.c_str());

    sendDiagnosticMessageDisplay(sendDisplay, hDisplay, payloadDisplay);
    return true;
}

bool readIfConfigFile()
{
    printf("PCDoIP::readIpConfigFile, start\n");
    FILE* fp = fopen(CONFIG_FILE_PATH, "r");
    if (NULL == fp) {
        printf("PCDoIP::readIpConfigFile, fopen failed\n");
        return false;
    }

    char lineBuf[64] = { 0 };
    char *tmp = NULL;
    int configNum = 0;
    while (NULL != fgets(lineBuf, sizeof(lineBuf) -1, fp)) {
        tmp = lineBuf;
        while (*tmp && isspace(*tmp)) {
            ++tmp;
        }
        
        if (strncmp(tmp, PC_INTERFACE_IP, strlen(PC_INTERFACE_IP)) == 0) {
            tmp = tmp + strlen(PC_INTERFACE_IP) + strlen("=");
            sscanf(tmp, "%64s", pcIp);
            printf("PCDoIP::readIpConfigFile, PC IP Addr [%s]\n", pcIp);
            configNum++;
            if (2 == configNum) {
                return true;
            }
            continue;
        }
        else if (strncmp(tmp, VEHICLE_INTERFACE_IP, strlen(VEHICLE_INTERFACE_IP)) == 0) {
            tmp = tmp + strlen(VEHICLE_INTERFACE_IP) + strlen("=");
            sscanf(tmp, "%64s", remoteVehicleIp);
            printf("PCDoIP::readIpConfigFile, Remote Vehicle IP Addr [%s]\n", remoteVehicleIp);
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

bool init()
{
    initDisplay(INIT_START);
    setRoutingActivation(false);

    if (!readIfConfigFile()) {
        printf("PCDoIP::init, read config file failed \n");
        return false;
    }

    if (!createUdpSocket()) {
        printf("PCDoIP::init, create udp socket failed\n");
        return false;
    }

    if (!createTcpSocket()) {
        printf("PCDoIP::init, create tcp socket failed\n");
        return false;
    }

    if (!createMulticastSocket(DOIP_MULTICAST_IPV6_ADDR, DOIP_PORT)) {
        printf("PCDoIP::init, create multicast socket failed\n");
        return false;
    }

    pthread_t pid;
    if(0 != pthread_create(&pid, NULL, onReceiveMessage, NULL)) {
        printf("PCDoIP::init, create receive thread failed\n");
        return false;
    }

    initDisplay(INIT_END);
    return true;
}


bool deinit()
{
    // close scoket
    close(multicastSfd);
    close(sfdVehicleIdentification);
    close(tcpClientSfd);
    return true;
}

// Display init info
void initDisplay(const std::string& info)
{
    printf("PCDoIP::initDisplay, start\n");
}

// CASE1, CASE2 Request Display
void requestDisplay(const struct SendInfoDisplay& info, const struct HeaderDisplay& header)
{
    printf("PCDoIP::requestDisplay, start\n");
}

// CASE1: Response Vehicle Identification Display
void responseVehicleIdentificationDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct VehicleIdentificationDisplay& payload)
{
    printf("PCDoIP::responseVehicleIdentificationDisplay, start\n");
}
// CASE1: Announcement Vehicle Identification Display
void announcementVehicleIdentificationDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct VehicleIdentificationDisplay& payload)
{
    printf("PCDoIP::announcementVehicleIdentificationDisplay, start\n");
}

// CASE2: Response Entity Status Display
void responseDoipEntityStatusDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct EntityStatusDisplay& payload)
{
    printf("PCDoIP::responseDoipEntityStatusDisplay, start\n");
}

// CASE3: Request Routing Activation Display
void reqeustRoutingActivationoDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct RoutingActivationRequestDisplay& payload)
{
    printf("PCDoIP::reqeustRoutingActivationoDisplay, start\n");
}

// CASE3: Response Routing Activation Display
void responseRoutingActivationoDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct RoutingActivationResponseDisplay& payload)
{
    printf("PCDoIP::responseRoutingActivationoDisplay, start\n");
}

// CASE4: Befor you can send diagnostic message, you need press [Routeing Activation] button
void needRoutingActivationDisplay(const std::string& needInfo)
{
    printf("PCDoIP::needRoutingActivationDisplay, start\n");
}

// CASE4: Send Diagnostic Display
void sendDiagnosticMessageDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct DiagnosticMessageDisplay& payload)
{
    printf("PCDoIP::sendDiagnosticMessageDisplay, start\n");
}

// CASE4: Response Diagnostic Ack Display
void responseDiagnosticMessageAckDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct DiagnosticMessageAckDisplay& payload)
{
    printf("PCDoIP::responseDiagnosticMessageAckDisplay, start\n");
}

int main()
{
    printf("PCDoIP::main, start\n");
    if (!init()) {
        printf("PCDoIP::main, init failed\n");
        return -1;
    }

    sleep(5);

    if (!requestVehicleIdentification()) {
        printf("PCDoIP::main, requestVehicleIdentification failed\n");
        return -1;
    }

    sleep(5);

    if(!requestEntityStatus()) {
        printf("PCDoIP::main, requestEntityStatu failed\n");
        return -1; 
    }

    sleep(5);

    if(!requestRoutingActivation()) {
        printf("PCDoIP::main, requestRoutingActivation failed\n");
        return -1;
    }

    sleep(5);

    if(!sendDiagnosticMessage()) {
        printf("PCDoIP::main, sendDiagnosticMessage failed\n");
        return -1;
    } 

    sleep(100000);

    deinit();
    return 0;
}








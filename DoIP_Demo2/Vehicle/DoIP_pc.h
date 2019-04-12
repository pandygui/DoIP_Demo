#ifndef _DOIP_
#define _DOIP_

#include <stdint.h>
#include <string>

#define INIT_START "Init detection equipment start"
#define INIT_END   "Init detection equipment success"

#define NEED_ROUTING_ACTIVATION "Befor you can send diagnostic message, you need press [Routeing Activation] button"

struct SendInfoDisplay
{
    std::string srcIp;
    std::string srcPort;
    std::string dstIp;
    std::string dstPort;
};


struct HeaderDisplay
{
    std::string protocolVersion;
    std::string inverseProtocolVersion;
    std::string payloadType;
    std::string payloadLength;
};

// CASE1 Response Info
struct VehicleIdentificationDisplay
{
    std::string vin;
    std::string entityLA;
    std::string eid;
    std::string gid;
    std::string furtherAction;
    std::string syncStatus; 
};

// CASE2 Response Info
struct EntityStatusDisplay
{
    std::string nodeType;
    std::string maxTcpSockets;
    std::string currentTcpSockets;
    std::string maxDataSize;
};

// CASE3 Request Info
struct RoutingActivationRequestDisplay
{
    std::string externalLA;
    std::string activationType;
};

// CASE3 Response Info
struct RoutingActivationResponseDisplay
{
    std::string externalLA;
    std::string entityLA;
    std::string responseCode;
};

// CASE4 Request Info
struct DiagnosticMessageDisplay
{
    std::string externalLA;
    std::string entityLA;
    std::string userData;
};

// CASE4 Response Info
struct DiagnosticMessageAckDisplay
{
    std::string entityLA;
    std::string externalLA;
    std::string ackCode;
    std::string previousData;
};


// Start call
bool init();

// Stop call
bool deinit();

// CASE1 : DoIP Message : Vehicle Identification Request
bool requestVehicleIdentification();
// CASE2 : DoIP Message : DoIP Entity Status Request
bool requestEntityStatus();
// CASE3 : DoIP Message : TCP Routing Activation Request
bool requestRoutingActivation();
// CASE4 : DoIP Message : Diagnostic Request
bool sendDiagnosticMessage();

// Display init info
void initDisplay(const std::string& info);

// CASE1, CASE2 Request Display
void requestDisplay(const struct SendInfoDisplay& info, const struct HeaderDisplay& header);

// CASE1: Response Vehicle Identification Display
void responseVehicleIdentificationDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct VehicleIdentificationDisplay& payload);
// CASE1: Announcement Vehicle Identification Display
void announcementVehicleIdentificationDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct VehicleIdentificationDisplay& payload);

// CASE2: Response Entity Status Display
void responseDoipEntityStatusDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct EntityStatusDisplay& payload);

// CASE3: Request Routing Activation Display
void reqeustRoutingActivationoDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct RoutingActivationRequestDisplay& payload);
// CASE3: Response Routing Activation Display
void responseRoutingActivationoDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct RoutingActivationResponseDisplay& payload);

// CASE4: Befor you can send diagnostic message, you need press [Routeing Activation] button
void needRoutingActivationDisplay(const std::string& needInfo);
// CASE4: Send Diagnostic Display
void sendDiagnosticMessageDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct DiagnosticMessageDisplay& payload);
// CASE4: Response Diagnostic Ack Display
void responseDiagnosticMessageAckDisplay(const struct SendInfoDisplay& sendInfo, 
    const struct HeaderDisplay& header, const struct DiagnosticMessageAckDisplay& payload);

#define DOIP_MULTICAST_IPV6_ADDR "ff02::1:5"
#define DOIP_PORT 13400

#define DOIP_SOCKET_RECV_TIMEOUT 1

#define ANNOUNCEMENT_VEHICLE_IDENTIFICATION_INTERVAL 500*1000
#define ANNOUNCEMENT_VEHICLE_IDENTIFICATION_TIMES 3

// Define DoIP header
#define DOIP_HEADER_SIZE 0x08

#define DOIP_HEADER_PROTOCOL_VERSION 0x01
#define DOIP_HEADER_INVERSE_PROTOCOL_VERSION 0xFE

#define PAYLOAD_TYPE_REQUEST_VEHICLE_IDENTIFICATION 0x0001
#define PAYLOAD_TYPE_RESPONSE_VEHICLE_IDENTIFICATION 0x0004
#define PAYLOAD_TYPE_ANNOUNCEMENT_VEHICLE_IDENTIFICATION 0x0009

#define PAYLOAD_TYPE_REQUEST_DIAGNOSTIC_POWER_MODE 0x4003
#define PAYLOAD_TYPE_RESPONSE_DIAGNOSTIC_POWER_MODE 0x4004

#define PAYLOAD_TYPE_REQUEST_ENTITY_STATUS 0x4001
#define PAYLOAD_TYPE_RESPONSE_ENTITY_STATUS 0x4002

#define PAYLOAD_TYPE_REQUEST_ROUTING_ACTIVATION 0x0005
#define PAYLOAD_TYPE_REPONSE_ROUTING_ACTIVATION 0x0006

#define PAYLOAD_TYPE_SEND_DIAGNOSTIC_MESSAGE 0x8001
#define PAYLOAD_TYPE_POSITIVE_ACK_DIAGNOSTIC_MESSAGE 0x8002

// DoIP Payload
#define NODE_TYPE_DOIP_NODE 0x01
#define NODE_TYPE_DOIP_GATEWAY 0x00

#define DOIP_MAX_TCP_SOCKET 0x04
#define DOIP_IPV6_ADDR_LEN 64
#define DOIP_UDP_MESSAGE_MAX_SIZE 1024
#define IFNAME_MAX_LEN 16

#define CONFIG_FILE_PATH "/tmp/config"
#define VEHICLE_INTERFACE_NAME "vehicle_ifname"
#define VEHICLE_INTERFACE_IP "vehicle_ipaddr"
#define PC_INTERFACE_NAME "pc_ifname"
#define PC_INTERFACE_IP "pc_ipaddr"

#define OTHER_HEX 3
#define UINT8_HEX 4
#define UINT16_HEX 8
#define UINT32_HEX 16


struct DoIPHeader
{
    uint8_t protocolVersion;
    uint8_t inverseProtocolVersion;
    uint16_t payloadType;
    uint32_t payloadLength;
};

struct VehicleIdentificationResponse
{
    struct DoIPHeader header;
    uint8_t vin[17];
    uint16_t entityLA;
    uint8_t eid[6];
    uint8_t gid[6];
    uint8_t furtherAction;
    uint8_t syncStatus;
};

struct EntityStatusResponse
{
    struct DoIPHeader header;
    uint8_t nodeType;
    uint8_t maxTcpSockets;
    uint8_t currentTcpSockets;
    uint32_t maxDataSize;
};

struct RoutingActivationRequest
{
    struct DoIPHeader header;
    uint16_t externalLA;
    uint8_t activationType;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct RoutingActivationResponse
{
    struct DoIPHeader header;
    uint16_t externalLA;
    uint16_t entityLA;
    uint8_t responseCode;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct DiagnosticMessageRequest
{
    struct DoIPHeader header;
    uint16_t externalLA;
    uint16_t entityLA;
    uint16_t userData;
};

struct DiagnosticMessagePositiveAck
{
    struct DoIPHeader header;
    uint16_t entityLA;
    uint16_t externalLA;
    uint8_t ackCode;
    uint16_t previousData;
};

#endif // _DOIP_





#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	port		= onPort;
	clientMax	= maxClients;
	clientCount = 0;
	netHandle	= nullptr;
	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

// 初始化游戏服务器
bool GameServer::Initialise() {
    ENetAddress address;  // 创建一个 ENet 地址对象

    // 设置服务器监听地址和端口
    address.host = ENET_HOST_ANY;  // 设置服务器监听所有可用的网络接口
    address.port = port;           // 设置服务器监听的端口号（`port` 是类成员）

    // 创建一个 ENet 主机（服务器），并设置最大客户端连接数为 `clientMax`
    netHandle = enet_host_create(&address, clientMax, 1, 0, 0);
    if (enet_initialize() != 0) {
        std::cout << "ENet failed to initialize!" << std::endl;
        return false;
    }
    // 检查主机创建是否成功
    if (!netHandle) {
        std::cout << __FUNCTION__ << " failed to create network handle!" << std::endl;
        return false;  // 如果失败，返回 false
    }

    // 如果成功，返回 true
    return true;
}


// 发送全局消息包（仅使用消息 ID）
bool GameServer::SendGlobalPacket(int msgID) {
    GamePacket packet;            // 创建一个 GamePacket 对象
    packet.type = msgID;          // 设置消息包的类型（ID）
    return SendGlobalPacket(packet); // 调用另一个重载的 SendGlobalPacket 函数，传递整个包对象
}

// 发送全局消息包（完整的 GamePacket 对象）
bool GameServer::SendGlobalPacket(GamePacket& packet) {
    // 创建一个 ENetPacket 对象，并将 GamePacket 的数据作为负载
    ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);

    // 广播数据包到所有连接的客户端
    enet_host_broadcast(netHandle, 0, dataPacket);

    // 返回成功
    return true;
}

//bool GameServer::SendGlobalPacket(StringPacket& packet) {
    // 创建一个 ENetPacket 对象，并将 GamePacket 的数据作为负载
    //ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);

    // 广播数据包到所有连接的客户端
    //enet_host_broadcast(netHandle, 0, dataPacket);

    // 返回成功
    //return true;
//}


void GameServer::UpdateServer() {
    if (!netHandle) {
        return;  // 如果网络句柄为空，直接返回
    }

    ENetEvent event;  // 创建 ENetEvent 对象来处理事件
    while (enet_host_service(netHandle, &event, 0) > 0) {  // 等待事件并处理
        int type = event.type;  // 获取事件类型
        ENetPeer* p = event.peer;  // 获取与事件相关的 ENetPeer（表示与客户端的连接）
        int peer = p->incomingPeerID;  // 获取对等端（客户端）的 ID

        // 处理连接事件
        if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Server: New client connected" << std::endl;
        }
        // 处理断开连接事件
        else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
            std::cout << "Server: A client has disconnected" << std::endl;
        }
        // 处理接收数据包事件
        else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
            GamePacket* packet = (GamePacket*)event.packet->data;  // 获取接收到的包
            ProcessPacket(packet, peer);  // 处理接收到的包
        }

        enet_packet_destroy(event.packet);  // 销毁处理完的 ENetPacket，释放内存
    }
}


void GameServer::SetGameWorld(GameWorld &g) {
	gameWorld = &g;
}
#include "GameClient.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

// 连接到指定 IP 地址和端口号的服务器
bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
    ENetAddress address;              // 定义 ENet 地址结构体
    address.port = portNum;           // 设置目标端口号

    // 通过 IP 地址的四个字节计算出 32 位的主机地址
    address.host = (d << 24) | (c << 16) | (b << 8) | (a);

    // 尝试连接到指定地址和端口的服务器
    netPeer = enet_host_connect(netHandle, &address, 2, 0);

    // 返回连接是否成功
    return netPeer != nullptr;
}


// 更新客户端网络状态并处理事件
void GameClient::UpdateClient() {
    // 如果没有有效的网络句柄，直接返回
    if (netHandle == nullptr) {
        return;
    }

    ENetEvent event; // 定义 ENet 事件对象

    // 处理所有接收到的网络事件
    while (enet_host_service(netHandle, &event, 0) > 0) {
        // 处理连接事件
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connected to server!" << std::endl;
        }
        // 处理接收数据事件
        else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            std::cout << "Client: Packet received..." << std::endl;

            // 将接收到的 ENet 数据包转换为游戏数据包
            GamePacket* packet = reinterpret_cast<GamePacket*>(event.packet->data);

            // 调用自定义的处理函数对数据包进行处理
            ProcessPacket(packet);
        }

        // 销毁当前事件的数据包（释放内存）
        enet_packet_destroy(event.packet);
    }
}


// 发送数据包到服务器
void GameClient::SendPacket(GamePacket& payload) {
    // 创建 ENet 数据包
    ENetPacket* dataPacket = enet_packet_create(
        &payload,                   // 数据包的起始地址
        payload.GetTotalSize(),     // 数据包的总大小（字节数）
        0                           // 数据包的标志（0 表示无特殊标志）
    );

    // 通过当前连接的网络对等体 (netPeer) 发送数据包
    enet_peer_send(netPeer, 0, dataPacket);
}


#include "NetworkBase.h"
#include "./enet/enet.h"
NetworkBase::NetworkBase()	{
	netHandle = nullptr;
}

NetworkBase::~NetworkBase()	{
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

// 处理网络数据包的函数
bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
    PacketHandlerIterator firstHandler;
    PacketHandlerIterator lastHandler;

    // 获取指定类型的数据包处理器范围
    bool canHandle = GetPacketHandlers(packet->type, firstHandler, lastHandler);

    // 如果找到可以处理该数据包的处理器
    if (canHandle) {
        for (auto i = firstHandler; i != lastHandler; ++i) {
            // 调用处理器的 ReceivePacket 方法来处理数据包
            i->second->ReceivePacket(packet->type, packet, peerID);
        }
        return true; // 数据包已成功处理
    }

    // 如果没有找到适合的数据包处理器，打印错误信息
    std::cout << __FUNCTION__ << " no handler for packet type "
        << packet->type << std::endl;

    return false; // 数据包未能处理
}

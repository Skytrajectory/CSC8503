#pragma once
//#include "./enet/enet.h"
struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

enum BasicNetworkMessages {
	None,
	Hello,
	Message,
	String_Message,
	Delta_State,	//1 byte per channel since the last state
	Full_State,		//Full transform etc
	Received_State, //received from a client, informs that its received packet n
	Player_Connected,
	Player_Disconnected,
	Shutdown,
	Player_Score
};

struct GamePacket {
	short size;
	short type;

	GamePacket() {
		type		= BasicNetworkMessages::None;
		size		= 0;
	}

	GamePacket(short type) : GamePacket() {
		this->type	= type;
	}

	int GetTotalSize() {
		return sizeof(GamePacket) + size;
	}
};

// 定义一个继承自 GamePacket 的 StringPacket 结构体，用于处理字符串数据包
struct StringPacket : public GamePacket {
	char stringData[256];  // 用于存储字符串数据的字符数组

	// 构造函数，接受一个 std::string 类型的消息并填充数据包
	StringPacket(const std::string& message) {
		type = BasicNetworkMessages::String_Message;  // 设置数据包的类型为 String_Message
		size = (short)message.length();  // 设置数据包大小为消息的长度
		memcpy(stringData, message.data(), size);  // 将消息数据复制到 stringData 中
	}

	// 从数据中获取字符串，返回一个 std::string
	std::string GetStringFromData() {
		std::string realString(stringData);  // 使用 stringData 创建 std::string
		realString.resize(size);  // 调整字符串大小以确保正确
		return realString;  // 返回处理后的字符串
	}
	//GamePacket* GetStringFromData() {
		//return this;  // 返回处理后的字符串
	//}
};

struct ScorePacket : public GamePacket {
	int playerscore;  // 用于存储字符串数据的字符数组

	// 构造函数，接受一个 std::string 类型的消息并填充数据包
	ScorePacket(int s) {
		type = Player_Score;
		size= sizeof(ScorePacket)-sizeof(GamePacket);  // 设置数据包大小为消息的长度
		playerscore = s;  // 设置数据包大小为消息的长度
	}
};

//struct Score : public GamePacket {
//	int playerscore;  // 用于存储字符串数据的字符数组
//
//	// 构造函数，接受一个 std::string 类型的消息并填充数据包
//	Score(int s) {
//		type =
//			playerscore = s;  // 设置数据包大小为消息的长度
//	}
//};

class PacketReceiver {
public:
	virtual void ReceivePacket(int type, GamePacket* payload, int source = -1) = 0;
};

class NetworkBase	{
public:
	static void Initialise();
	static void Destroy();

	static int GetDefaultPort() {
		return 1234;
	}

	void RegisterPacketHandler(int msgID, PacketReceiver* receiver) {
		packetHandlers.insert(std::make_pair(msgID, receiver));
	}
protected:
	NetworkBase();
	~NetworkBase();

	bool ProcessPacket(GamePacket* p, int peerID = -1);

	typedef std::multimap<int, PacketReceiver*>::const_iterator PacketHandlerIterator;

	bool GetPacketHandlers(int msgID, PacketHandlerIterator& first, PacketHandlerIterator& last) const {
		auto range = packetHandlers.equal_range(msgID);

		if (range.first == packetHandlers.end()) {
			return false; //no handlers for this message type!
		}
		first	= range.first;
		last	= range.second;
		return true;
	}

	_ENetHost* netHandle;

	std::multimap<int, PacketReceiver*> packetHandlers;
};
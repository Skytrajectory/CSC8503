#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o)	{
	deltaErrors = 0;
	fullErrors  = 0;
	networkID   = id;
}

NetworkObject::~NetworkObject()	{
}

/**
 * @brief 读取数据包
 *
 * 根据数据包的类型，调用对应的读取函数来处理全状态包或增量状态包。
 *
 * @param p 引用传递的 `GamePacket`，表示接收到的数据包。
 * @return true 如果成功读取数据包；false 如果数据包类型无效或不支持。
 */
bool NetworkObject::ReadPacket(GamePacket& p) {
    // 如果是增量状态包，调用 ReadDeltaPacket 处理
    if (p.type == Delta_State) {
        return ReadDeltaPacket((DeltaPacket&)p);
    }

    // 如果是全状态包，调用 ReadFullPacket 处理
    if (p.type == Full_State) {
        return ReadFullPacket((FullPacket&)p);
    }

    // 其他类型的数据包不处理，返回 false
    return false;
}


bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
	}
	return WriteFullPacket(p);
}
//Client objects recieve these packets
/**
 * @brief 读取增量状态包
 *
 * 根据增量状态包的信息更新对象的全局位置和方向。
 *
 * @param p 引用传递的 `DeltaPacket`，表示增量状态数据包。
 * @return true 如果成功应用增量状态更新；false 如果状态包不匹配或无法更新。
 */
bool NetworkObject::ReadDeltaPacket(DeltaPacket& p) {
    // 检查状态ID是否与上一次的全状态ID匹配，不匹配则无法应用增量状态
    if (p.fullID != lastFullState.stateID) {
        return false; // 无法应用此帧的增量状态
    }

    // 更新状态历史
    UpdateStateHistory(p.fullID);

    // 获取上一次全状态的位置信息和方向信息
    Vector3 fullPos = lastFullState.position;
    Quaternion fullOrientation = lastFullState.orientation;

    // 根据增量包的信息更新位置
    fullPos.x += p.pos[0];
    fullPos.y += p.pos[1];
    fullPos.z += p.pos[2];

    // 根据增量包的信息更新方向
    fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
    fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
    fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
    fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

    // 应用新的位置和方向到对象
    object.GetTransform().SetPosition(fullPos);
    object.GetTransform().SetOrientation(fullOrientation);

    return true;
}


/**
 * @brief 读取完整状态包
 *
 * 更新对象的全局位置和方向到完整状态包所提供的信息。
 *
 * @param p 引用传递的 `FullPacket`，表示完整状态数据包。
 * @return true 如果成功应用完整状态更新；false 如果数据包的状态ID已过期。
 */
bool NetworkObject::ReadFullPacket(FullPacket& p) {
    // 检查完整状态包的状态ID是否比当前最新状态旧，若旧则忽略该包
    if (p.fullState.stateID < lastFullState.stateID) {
        return false; // 收到过期的数据包，忽略
    }

    // 更新最后的完整状态为当前数据包的状态
    lastFullState = p.fullState;

    // 设置对象的全局位置和方向
    object.GetTransform().SetPosition(lastFullState.position);
    object.GetTransform().SetOrientation(lastFullState.orientation);

    // 将新的完整状态记录到状态历史中
    stateHistory.emplace_back(lastFullState);

    return true; // 成功应用完整状态更新
}


/**
 * @brief 写入增量数据包
 *
 * 此函数生成一个 `DeltaPacket`，包含相对于某个状态的增量信息（位置和朝向的变化）。
 * 如果无法获取对应状态，则返回 false。
 *
 * @param p 双指针，用于存储生成的 `GamePacket`。
 * @param stateID 指定的状态ID，用于计算增量。
 * @return true 如果成功生成增量包；false 如果失败。
 */
bool NetworkObject::WriteDeltaPacket(GamePacket** p, int stateID) {
    DeltaPacket* dp = new DeltaPacket(); // 创建增量数据包
    NetworkState state;

    // 尝试获取指定状态的信息，失败则返回 false
    if (!GetNetworkState(stateID, state)) {
        return false; // 无法生成增量包
    }

    dp->fullID = stateID;         // 设置状态ID
    dp->objectID = networkID;     // 设置网络对象ID

    // 获取当前的世界位置和朝向
    Vector3 currentPos = object.GetTransform().GetPosition();
    Quaternion currentOrientation = object.GetTransform().GetOrientation();

    // 计算相对于指定状态的增量
    currentPos -= state.position;
    currentOrientation -= state.orientation;

    // 将位置增量压缩到字符数组中（每个坐标值存储为 char 类型）
    dp->pos[0] = (char)currentPos.x;
    dp->pos[1] = (char)currentPos.y;
    dp->pos[2] = (char)currentPos.z;

    // 将朝向增量压缩到字符数组中（每个分量值映射到 [-127, 127]）
    dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
    dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
    dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
    dp->orientation[3] = (char)(currentOrientation.w * 127.0f);

    *p = dp; // 将生成的增量包指针赋值给 p
    return true; // 成功生成增量包
}



bool NetworkObject::WriteFullPacket(GamePacket** p) {
	FullPacket* fp = new FullPacket(); // 创建完整数据包

	fp->objectID = networkID; // 设置网络对象ID
	fp->fullState.position = object.GetTransform().GetPosition(); // 获取对象的世界位置
	fp->fullState.orientation = object.GetTransform().GetOrientation(); // 获取对象的世界朝向
	fp->fullState.stateID = lastFullState.stateID++; // 更新并设置状态ID

	*p = fp; // 将生成的数据包指针赋值给 p
	return true; // 成功写入返回 true
}


NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}
/**
 * @brief 获取特定状态ID的网络状态
 *
 * 从 `stateHistory` 中查找指定状态ID对应的状态。
 *
 * @param stateID 要查找的状态ID
 * @param state 用于保存查找到的状态
 * @return true 如果找到指定状态；false 如果未找到。
 */
bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
    for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
        if ((*i).stateID == stateID) {
            state = (*i); // 复制找到的状态
            return true;  // 找到状态
        }
    }
    return false; // 未找到状态
}
/**
 * @brief 更新状态历史，移除过期的状态
 *
 * 删除 `stateHistory` 中所有状态ID小于指定最小值的状态。
 *
 * @param minID 要保留的最小状态ID
 */
void NetworkObject::UpdateStateHistory(int minID) {
    for (auto i = stateHistory.begin(); i < stateHistory.end();) {
        if ((*i).stateID < minID) {
            i = stateHistory.erase(i); // 移除过期状态
        }
        else {
            ++i; // 保留有效状态
        }
    }
}
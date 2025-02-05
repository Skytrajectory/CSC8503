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
 * @brief ��ȡ���ݰ�
 *
 * �������ݰ������ͣ����ö�Ӧ�Ķ�ȡ����������ȫ״̬��������״̬����
 *
 * @param p ���ô��ݵ� `GamePacket`����ʾ���յ������ݰ���
 * @return true ����ɹ���ȡ���ݰ���false ������ݰ�������Ч��֧�֡�
 */
bool NetworkObject::ReadPacket(GamePacket& p) {
    // ���������״̬�������� ReadDeltaPacket ����
    if (p.type == Delta_State) {
        return ReadDeltaPacket((DeltaPacket&)p);
    }

    // �����ȫ״̬�������� ReadFullPacket ����
    if (p.type == Full_State) {
        return ReadFullPacket((FullPacket&)p);
    }

    // �������͵����ݰ����������� false
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
 * @brief ��ȡ����״̬��
 *
 * ��������״̬������Ϣ���¶����ȫ��λ�úͷ���
 *
 * @param p ���ô��ݵ� `DeltaPacket`����ʾ����״̬���ݰ���
 * @return true ����ɹ�Ӧ������״̬���£�false ���״̬����ƥ����޷����¡�
 */
bool NetworkObject::ReadDeltaPacket(DeltaPacket& p) {
    // ���״̬ID�Ƿ�����һ�ε�ȫ״̬IDƥ�䣬��ƥ�����޷�Ӧ������״̬
    if (p.fullID != lastFullState.stateID) {
        return false; // �޷�Ӧ�ô�֡������״̬
    }

    // ����״̬��ʷ
    UpdateStateHistory(p.fullID);

    // ��ȡ��һ��ȫ״̬��λ����Ϣ�ͷ�����Ϣ
    Vector3 fullPos = lastFullState.position;
    Quaternion fullOrientation = lastFullState.orientation;

    // ��������������Ϣ����λ��
    fullPos.x += p.pos[0];
    fullPos.y += p.pos[1];
    fullPos.z += p.pos[2];

    // ��������������Ϣ���·���
    fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
    fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
    fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
    fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

    // Ӧ���µ�λ�úͷ��򵽶���
    object.GetTransform().SetPosition(fullPos);
    object.GetTransform().SetOrientation(fullOrientation);

    return true;
}


/**
 * @brief ��ȡ����״̬��
 *
 * ���¶����ȫ��λ�úͷ�������״̬�����ṩ����Ϣ��
 *
 * @param p ���ô��ݵ� `FullPacket`����ʾ����״̬���ݰ���
 * @return true ����ɹ�Ӧ������״̬���£�false ������ݰ���״̬ID�ѹ��ڡ�
 */
bool NetworkObject::ReadFullPacket(FullPacket& p) {
    // �������״̬����״̬ID�Ƿ�ȵ�ǰ����״̬�ɣ���������Ըð�
    if (p.fullState.stateID < lastFullState.stateID) {
        return false; // �յ����ڵ����ݰ�������
    }

    // ������������״̬Ϊ��ǰ���ݰ���״̬
    lastFullState = p.fullState;

    // ���ö����ȫ��λ�úͷ���
    object.GetTransform().SetPosition(lastFullState.position);
    object.GetTransform().SetOrientation(lastFullState.orientation);

    // ���µ�����״̬��¼��״̬��ʷ��
    stateHistory.emplace_back(lastFullState);

    return true; // �ɹ�Ӧ������״̬����
}


/**
 * @brief д���������ݰ�
 *
 * �˺�������һ�� `DeltaPacket`�����������ĳ��״̬��������Ϣ��λ�úͳ���ı仯����
 * ����޷���ȡ��Ӧ״̬���򷵻� false��
 *
 * @param p ˫ָ�룬���ڴ洢���ɵ� `GamePacket`��
 * @param stateID ָ����״̬ID�����ڼ���������
 * @return true ����ɹ�������������false ���ʧ�ܡ�
 */
bool NetworkObject::WriteDeltaPacket(GamePacket** p, int stateID) {
    DeltaPacket* dp = new DeltaPacket(); // �����������ݰ�
    NetworkState state;

    // ���Ի�ȡָ��״̬����Ϣ��ʧ���򷵻� false
    if (!GetNetworkState(stateID, state)) {
        return false; // �޷�����������
    }

    dp->fullID = stateID;         // ����״̬ID
    dp->objectID = networkID;     // �����������ID

    // ��ȡ��ǰ������λ�úͳ���
    Vector3 currentPos = object.GetTransform().GetPosition();
    Quaternion currentOrientation = object.GetTransform().GetOrientation();

    // ���������ָ��״̬������
    currentPos -= state.position;
    currentOrientation -= state.orientation;

    // ��λ������ѹ�����ַ������У�ÿ������ֵ�洢Ϊ char ���ͣ�
    dp->pos[0] = (char)currentPos.x;
    dp->pos[1] = (char)currentPos.y;
    dp->pos[2] = (char)currentPos.z;

    // ����������ѹ�����ַ������У�ÿ������ֵӳ�䵽 [-127, 127]��
    dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
    dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
    dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
    dp->orientation[3] = (char)(currentOrientation.w * 127.0f);

    *p = dp; // �����ɵ�������ָ�븳ֵ�� p
    return true; // �ɹ�����������
}



bool NetworkObject::WriteFullPacket(GamePacket** p) {
	FullPacket* fp = new FullPacket(); // �����������ݰ�

	fp->objectID = networkID; // �����������ID
	fp->fullState.position = object.GetTransform().GetPosition(); // ��ȡ���������λ��
	fp->fullState.orientation = object.GetTransform().GetOrientation(); // ��ȡ��������糯��
	fp->fullState.stateID = lastFullState.stateID++; // ���²�����״̬ID

	*p = fp; // �����ɵ����ݰ�ָ�븳ֵ�� p
	return true; // �ɹ�д�뷵�� true
}


NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}
/**
 * @brief ��ȡ�ض�״̬ID������״̬
 *
 * �� `stateHistory` �в���ָ��״̬ID��Ӧ��״̬��
 *
 * @param stateID Ҫ���ҵ�״̬ID
 * @param state ���ڱ�����ҵ���״̬
 * @return true ����ҵ�ָ��״̬��false ���δ�ҵ���
 */
bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
    for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
        if ((*i).stateID == stateID) {
            state = (*i); // �����ҵ���״̬
            return true;  // �ҵ�״̬
        }
    }
    return false; // δ�ҵ�״̬
}
/**
 * @brief ����״̬��ʷ���Ƴ����ڵ�״̬
 *
 * ɾ�� `stateHistory` ������״̬IDС��ָ����Сֵ��״̬��
 *
 * @param minID Ҫ��������С״̬ID
 */
void NetworkObject::UpdateStateHistory(int minID) {
    for (auto i = stateHistory.begin(); i < stateHistory.end();) {
        if ((*i).stateID < minID) {
            i = stateHistory.erase(i); // �Ƴ�����״̬
        }
        else {
            ++i; // ������Ч״̬
        }
    }
}
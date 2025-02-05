#include "PositionConstraint.h"
//#include "../../Common/Vector3.h"
#include "GameObject.h"
#include "PhysicsObject.h"
//#include "Debug.h"



using namespace NCL;
using namespace Maths;
using namespace CSC8503;

PositionConstraint::PositionConstraint(GameObject* a, GameObject* b, float d)
{
	objectA		= a;
	objectB		= b;
	distance	= d;
}

PositionConstraint::~PositionConstraint()
{

}

//a simple constraint that stops objects from being more than <distance> away
//from each other...this would be all we need to simulate a rope, or a ragdoll
void PositionConstraint::UpdateConstraint(float dt) {
    // 计算两个物体之间的相对位置
    Vector3 relativePos = objectA->GetTransform().GetPosition() -
        objectB->GetTransform().GetPosition();

    // 计算当前距离
    float currentDistance = Vector::Length(relativePos);

    // 计算偏移量
    float offset = distance - currentDistance;

    // 如果偏移量非零，执行约束调整
    if (abs(offset) > 0.0f) {
        // 计算偏移方向
        Vector3 offsetDir = Vector::Normalise(relativePos);

        // 获取两个物体的物理对象
        PhysicsObject* physA = objectA->GetPhysicsObject();
        PhysicsObject* physB = objectB->GetPhysicsObject();

        // 计算相对速度
        Vector3 relativeVelocity = physA->GetLinearVelocity() -
            physB->GetLinearVelocity();

        // 计算约束质量（总的质量倒数）
        float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

        // 如果约束质量大于零，计算并施加冲量
        if (constraintMass > 0.0f) {
            // 计算相对速度在偏移方向上的分量
            float velocityDot = Vector::Dot(relativeVelocity, offsetDir);

            // 偏置因子用于解决数值稳定性问题
            float biasFactor = 0.01f;
            float bias = -(biasFactor / dt) * offset;

            // 计算拉格朗日乘数
            float lambda = -(velocityDot + bias) / constraintMass;

            // 计算两个物体受到的冲量
            Vector3 aImpulse = offsetDir * lambda;
            Vector3 bImpulse = -offsetDir * lambda;

            // 应用冲量以调整位置
            physA->ApplyLinearImpulse(aImpulse);
            physB->ApplyLinearImpulse(bImpulse);
        }
    }
}


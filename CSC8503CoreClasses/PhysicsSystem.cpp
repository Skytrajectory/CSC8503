#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "Quaternion.h"
#include "PositionConstraint.h"

#include "Constraint.h"

#include "Debug.h"
#include "Window.h"
#include <functional>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= false;
	useBroadPhase	= true;	
	dTOffset		= 0.0f;
	globalDamping	= 0.995f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/

bool useSimpleContainer = false;

int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the 
iteration count down until the FPS stabilises, even if that ends up
being at a low rate. 
*/
int realHZ		= idealHZ;
float realDT	= idealDT;

void PhysicsSystem::Update(float dt) {	
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::N)) {
		useSimpleContainer = !useSimpleContainer;
		std::cout << "Setting broad container to " << useSimpleContainer << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}
	int iteratorCount = 0;
	while(dTOffset > realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT /  (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		IntegrateVelocity(realDT); //update positions from new velocity changes

		dTOffset -= realDT;
		iteratorCount++;
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if(dt*2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

//void PhysicsSystem::UpdateObjectAABBs() {
	//gameWorld.OperateOnContents(
		//[](GameObject* g) {
			//g->UpdateBroadphaseAABB();
		//}
	//);
//}
void PhysicsSystem::UpdateObjectAABBs() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	// 获取游戏世界中的所有物体迭代器
	gameWorld.GetObjectIterators(first, last);

	// 遍历所有物体，更新每个物体的 AABB
	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();
	}
}


/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	// 获取游戏世界中的所有物体迭代器
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	// 遍历所有物体，进行两两碰撞检测
	for (auto i = first; i != last; ++i) {
		// 跳过没有物理对象的物体
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		for (auto j = i + 1; j != last; ++j) {
			// 跳过没有物理对象的物体
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;
			}

			// 创建碰撞信息对象
			CollisionDetection::CollisionInfo info;

			// 检测两个物体是否发生碰撞
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				// 输出碰撞信息
				std::cout << "Collision between " << (*i)->GetName()
					<< " and " << (*j)->GetName() << std::endl;

				if ((*i)->GetName() == "Kitten" && (*j)->GetName() == "Cat") {
					std::cout << "Catch";
					//PositionConstraint* constraint = new PositionConstraint(*i, *j, 30);
				}

				ImpulseResolveCollision(*info.a, *info.b, info.point);

				// 设置碰撞信息的持续帧数
				info.framesLeft = numCollisionFrames;

				// 将碰撞信息存储到碰撞集合中
				allCollisions.insert(info);
			}
		}
	}
}


/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 

*/
void PhysicsSystem::ImpulseResolveCollision(
	GameObject& a, GameObject& b,
	CollisionDetection::ContactPoint& p) const {

	// 获取两个物体的物理对象
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	// 获取两个物体的变换
	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	// 计算总质量的倒数
	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	// 如果两物体都是静态的，直接返回
	if (totalMass == 0) {
		return;
	}

	// 利用投影将两个物体分离
	transformA.SetPosition(
		transformA.GetPosition() -
		(p.normal * p.penetration * (physA->GetInverseMass() / totalMass))
	);

	transformB.SetPosition(
		transformB.GetPosition() +
		(p.normal * p.penetration * (physB->GetInverseMass() / totalMass))
	);

	// 计算碰撞点的相对位置
	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	// 计算角速度引起的速度
	Vector3 angVelocityA = Vector::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB = Vector::Cross(physB->GetAngularVelocity(), relativeB);

	// 计算物体的完整速度
	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	// 计算碰撞点的相对速度
	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	// 计算法向上的冲量
	float impulseForce = Vector::Dot(contactVelocity, p.normal);

	// 考虑惯性对碰撞的影响
	Vector3 inertiaA = Vector::Cross(
		physA->GetInertiaTensor() * Vector::Cross(relativeA, p.normal),
		relativeA
	);

	Vector3 inertiaB = Vector::Cross(
		physB->GetInertiaTensor() * Vector::Cross(relativeB, p.normal),
		relativeB
	);

	float angularEffect = Vector::Dot(inertiaA + inertiaB, p.normal);

	// 计算恢复系数，用于模拟动能的损耗
	float cRestitution = 0.66f;

	// 计算冲量值
	float j = (-(1.0f + cRestitution) * impulseForce) /
		(totalMass + angularEffect);

	// 计算总冲量向量
	Vector3 fullImpulse = p.normal * j;

	// 应用线性冲量
	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);

	// 应用角动量冲量
	physA->ApplyAngularImpulse(Vector::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector::Cross(relativeB, fullImpulse));
}


/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/
void PhysicsSystem::BroadPhase() {
	// 清空之前的宽相位碰撞检测结果
	broadphaseCollisions.clear();

	// 创建四叉树，用于优化潜在碰撞对象的分组检测
	QuadTree<GameObject*> tree(Vector2(1024, 1024), 7, 6);

	// 获取世界中的游戏对象的迭代器范围
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	// 将所有对象插入四叉树
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;

		// 如果对象没有宽相位 AABB，则跳过
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}

		// 获取对象位置并插入四叉树
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree.Insert(*i, pos, halfSizes);
	}

	// 遍历四叉树的内容，处理可能的碰撞对
	tree.OperateOnContents(
		[&](std::list<QuadTreeEntry<GameObject*>>& data) {
			CollisionDetection::CollisionInfo info;

			// 在当前四叉树节点中检查对象对的碰撞
			for (auto i = data.begin(); i != data.end(); ++i) {
				for (auto j = std::next(i); j != data.end(); ++j) {
					// 确保碰撞对按照指针顺序插入，避免重复
					info.a = std::min((*i).object, (*j).object);
					info.b = std::max((*i).object, (*j).object);

					// 将碰撞信息添加到宽相位碰撞集中
					broadphaseCollisions.insert(info);
				}
			}
		});
}


/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	// 遍历宽相位筛选出的潜在碰撞对
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = broadphaseCollisions.begin();
		i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;

		// 检查两个对象是否实际发生了碰撞
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			//std::cout << "Collision between (NarrowPhase)" << (info.a)->GetName()
			//	<< " and " << (info.b)->GetName() << std::endl;
			// 如果有碰撞，设置碰撞帧数倒计时
			info.framesLeft = numCollisionFrames;

			if (collisionCallback) {
				collisionCallback(info.a, info.b);
			}
			// 使用冲量解决碰撞
			ImpulseResolveCollision(*info.a, *info.b, info.point);

			// 将碰撞信息插入所有碰撞集合
			allCollisions.insert(info);


		}
	}
}


/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	// 获取游戏对象的迭代器，用于遍历所有的GameObject
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	// 遍历所有的GameObject
	for (auto i = first; i != last; ++i) {
		// 获取当前GameObject的PhysicsObject（物理对象）
		PhysicsObject* object = (*i)->GetPhysicsObject();

		// 如果当前GameObject没有物理对象，跳过
		if (object == nullptr) {
			continue; // No physics object for this GameObject!
		}

		// 获取物理对象的反质量值
		float inverseMass = object->GetInverseMass();

		// 获取物理对象的线速度和作用力
		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();

		// 计算加速度 = 作用力 / 质量（加速 = 力 * 反质量）
		Vector3 accel = force * inverseMass;

		// 如果启用了重力并且物体的质量大于零，则应用重力加速度
		if (applyGravity && inverseMass > 0) {
			accel += gravity; // don't move infinitely heavy things
		}

		// 更新物体的线速度 = 当前线速度 + 加速度 * 时间步长
		linearVel += accel * dt;

		// 设置物体的新的线速度
		object->SetLinearVelocity(linearVel);
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();
		object->UpdateInertiaTensor();
		Vector3 angAccel = object->GetInertiaTensor() * torque;
		angVel += angAccel * dt;
		object->SetAngularVelocity(angVel);
	}
}


/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	// 获取游戏对象的迭代器，用于遍历所有的GameObject
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	// 计算每帧的线性阻尼值
	float frameLinearDamping = 1.0f - (0.4f * dt);

	// 遍历所有的GameObject
	for (auto i = first; i != last; ++i) {
		// 获取当前GameObject的PhysicsObject（物理对象）
		PhysicsObject* object = (*i)->GetPhysicsObject();

		// 如果当前GameObject没有物理对象，跳过
		if (object == nullptr) {
			continue;
		}


		// 获取当前GameObject的Transform（变换）
		Transform& transform = (*i)->GetTransform();

		// 处理位置更新部分
		Vector3 position = transform.GetPosition();  // 获取当前物体的位置
		Vector3 linearVel = object->GetLinearVelocity();  // 获取物体的线速度

		// 更新物体的位置：位置 += 线速度 * 时间步长
		position += linearVel * dt;

		// 设置新的位置
		transform.SetPosition(position);

		// 处理线性阻尼部分
		linearVel = linearVel * frameLinearDamping;  // 应用阻尼，减少线速度
		object->SetLinearVelocity(linearVel);  // 设置更新后的线速度

		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();
		orientation = orientation + (Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();
		transform.SetOrientation(orientation);

		float frameAngularDamping = 1.0f - (0.4f * dt);
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);
	}
}


/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	gameWorld.OperateOnContents(
		[](GameObject* o) {
			o->GetPhysicsObject()->ClearForces();
		}
	);
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}
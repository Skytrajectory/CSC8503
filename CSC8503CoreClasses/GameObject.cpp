#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8503;

GameObject::GameObject(const std::string& objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Quaternion::RotationMatrix<Matrix3>(transform.GetOrientation());
		mat = Matrix::Absolute(mat);
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}


void GameObject::CalculatePath() {
	pathToTarget.clear();
	NavigationPath outPath;
	if (target == nullptr) return;
	bool found = grid->FindPath(GetTransform().GetPosition(), target->GetTransform().GetPosition(), outPath);
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pathToTarget.push_back(pos);
	}
}

void GameObject::MoveToTarget(float dt) {
	std::cout << "move move move";
	//if (pathToTarget.size() > 0) {
	//	auto it = pathToTarget.begin();
	//	Vector3 target = *it;
	//	Vector3 dir = (target - this->GetTransform().GetPosition());
	//	dir = Vector3(dir.x, 0, dir.z);
	//	dir = Vector::Normalise(dir);
	//	GetPhysicsObject()->SetLinearVelocity(dir * 500.0f * dt);

	//	if (dir.Length() <= 2.0f) {
	//		pathToTarget.erase(it);
	//		GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	//	}
	//}
}
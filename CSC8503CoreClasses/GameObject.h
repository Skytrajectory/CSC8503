#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "NavigationGrid.h"
#include "StateTransition.h"
#include "State.h"
#include "StateMachine.h"
using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		void SetActive(bool active) {
			isActive = active;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		void SetName(const std::string& newName) {
			name = newName;
		}

		void MoveToTarget(float dt);



		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		void SetStateMachine(StateMachine* sm) {
			stateMachine = sm;
		}

		void SetStateMachine2(StateMachine* sm) {
			stateMachine2 = sm;
		}

		StateMachine* GetStateMachine() const {
			return stateMachine;
		}

		StateMachine* GetStateMachine2() const {
			return stateMachine2;
		}

		void CalculatePath();

		int		GetWorldID() const {
			return worldID;
		}
		GameObject* target;
		NavigationGrid* grid;
		std::vector<Vector3> pathToTarget;
		float counter;

		StateMachine* stateMachine;
		StateMachine* stateMachine2;


	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		int			worldID;
		std::string	name;
		Vector3 broadphaseAABB;
	};
}


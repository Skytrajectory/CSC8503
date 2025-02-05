#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "EnemyObject.h"
#include "StateGameObject.h"
#include "NavigationGrid.h"
#include "GameServer.h"
#include "GameClient.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();
			void InitMenu(int selectedIndex);
			void PauseMenu();
			void InitCamera();
			void InitTimer();
			void StartGame(int mode);

			void InitWorld();
			virtual void UpdateGame(float dt);
			void RotateObjectTowards(GameObject* object, const Vector3& targetDirection);

		protected:
			void InitialiseAssets();

			void UpdateKeys();


			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void BridgeConstraintTest();
			void ConnectKittenCat();
			void InitCollisionCallback();
			void InitMazeWorld();
			void displayMainMenu(float dt);
			void displayPauseMenu();
			void displayGamingMenu();
			void UpdateEnemyMovement(GameObject* enemy, float dt);
			void UpdateFriendMovement(GameObject* goat, float dt);
			void UpdateGameTree(float dt);
			
			void InitEnemyStateMachine(GameObject* enemy, GameObject* player);
			void InitKeeperStateMachine(GameObject* enemy, GameObject* player, GameObject* keyObject);
			void InitFriendAIbehavior();

			void UpdateGameTree();
			void AddFriend();
			
			
			void HandleCollision(GameObject* a, GameObject* b);
			void TestNetworking();
			
			bool gameWin = false;
			bool gameLose = false;
			bool superPowerActive = false;
			bool hasKey = false;

			bool isServer = false;
			bool isClient = false;
			
			PerspectiveCamera* cameraMain;

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddDoorToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			
			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddNetworkPlayerToWorld(const Vector3& position);
			GameObject* AddFriendToWorld(const Vector3& position);


			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddKeeperToWorld(const Vector3& position);

			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddKittenToWorld(const Vector3& position);
			GameObject* AddHomeToWorld(const Vector3& position);
			GameObject* AddKeyToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);

			
			//GameEnemyObject* AddGameEnemyObject(const Vector3& position);
			GameObject* catBonus;
			GameObject* keyObject;
			GameObject* DoorObject;
			GameObject* FriendObject;

			GameObject* NetworkPlayerObject;


			GameObject* networkPlyerObject;

			
			std::vector<GameObject*> kittens;
			NavigationGrid* grid;

			GameObject* enemyObject;
			GameObject* keeperObject;


			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;
			// 在 TutorialGame 中添加一个成员变量来存储小猫与大猫的约束关系
			std::unordered_map<GameObject*, GameObject*> kittenConstraints;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;
			bool mainMenuShow;
			bool menu1;
			bool menu2;
			bool pauseMenu;

			int removedKittens = 0;
			int mainMenuIndex = 0;
			//path
			std::vector<Vector3> pathToFollow;
			std::vector<Vector3> pathToFollow2;

			int currentPathIndex = 0;
			int currentPathIndex2 = 0;

			bool movingForward = true;

			//time
			float gameCurrentTime = 0;
			float gameTime = 120.0f;
			BehaviourState state;
			BehaviourSequence* sequence;
			BehaviourSequence* rootSequence;
			BehaviourSelector* selection;

			std::string debugMessage;
			
			float		forceMagnitude;
			float angleYaw;
			float anglePitch;
			int kittenTotalNum = 3;
			int kittenGotNum = 0;
			float gameScore;

			GameObject* selectionObject = nullptr;
			GameObject* PlayerObject = nullptr;

			//GameEnemyObject* EnemyObject = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;
			Mesh* coinMesh = nullptr;
			Mesh* goat = nullptr;
			Mesh* gooseMesh = nullptr;

			
			
			Texture*	basicTex	= nullptr;
			Texture* keyTex = nullptr;

			GameServer* server;
			GameClient* client;

			Texture* DefaultTex = nullptr;

			
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	catMesh		= nullptr;
			Mesh*	kittenMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 10, -20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest;

		};
	}
}


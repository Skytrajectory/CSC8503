#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "PhysicsSystem.h"
#include "EnemyObject.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "Maths.h"

#include "GameServer.h"
#include "GameClient.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

#define M_PI 3.14159265358979323846


using namespace NCL;
using namespace CSC8503;

// 测试数据包接收器类，继承自 PacketReceiver 用于处理接收到的数据包
class TestPacketReceiver : public PacketReceiver {
public:
	// 构造函数，初始化接收器名称
	TestPacketReceiver(std::string name) {
		this->name = name;  // 将传入的名称保存为类成员变量
	}

	// 接收数据包并处理
	void ReceivePacket(int type, GamePacket* payload, int source) override {
		// 判断包类型是否为字符串消息类型
		if (type == String_Message) {
			// 将数据包强制转换为 StringPacket 类型
			StringPacket* realPacket = (StringPacket*)payload;

			// 获取数据包中的字符串内容
			std::string msg = realPacket->GetStringFromData();

			// 输出接收到的消息和发送者名称
			std::cout << name << " 2121received message: " << msg << std::endl;
		}
		if (type == Player_Score) {



			// 输出接收到的消息和发送者名称
			std::cout  << " goal: " << std::endl;
		}
	}

protected:
	std::string name;  // 存储接收器的名称
};

class ScorePacketReceiver : public PacketReceiver {
public:

	// 接收数据包并处理
	void ReceivePacket(int type, GamePacket* payload, int source) override {
		// 判断包类型是否为字符串消息类型
		if (type == Player_Score) {
			// 将数据包强制转换为 StringPacket 类型
			ScorePacket* realPacket = (ScorePacket*)payload;

			// 输出接收到的消息和发送者名称
			std::cout << " received message: " << realPacket << std::endl;
		}
	}

protected:

};

// 测试网络连接和数据包通信的功能
void TutorialGame::TestNetworking() {
	// 初始化网络基础设施
	NetworkBase::Initialise();

	// 创建测试包接收器，分别为服务器和客户端
	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	// 获取默认端口号
	int port = NetworkBase::GetDefaultPort();
	std::cout << "port:" << port << std::endl;

	// 创建服务器和客户端实例
	server = new GameServer(port, 1);  // 服务器绑定到指定端口，最多支持1个客户端
	client = new GameClient();  // 创建一个客户端

	// 注册包处理器，将字符串消息类型与接收器绑定
	server->RegisterPacketHandler(String_Message, &serverReceiver);  // 服务器注册处理 String_Message 的回调
	client->RegisterPacketHandler(String_Message, &clientReceiver);  // 客户端注册处理 String_Message 的回调

	//GameObject* cat = AddNetworkPlayerToWorld(Vector3(2, 0, 0));
	//NetworkObject serverCat(*cat, 1);  // 服务器端的小猫
	//NetworkObject clientCat(*cat, 1);  // 客户端的小猫
	// 客户端尝试连接到服务器
	bool canConnect = client->Connect(127, 0, 0, 1, port);  // 连接到本地服务器

	// 如果连接成功，则进行后续的通信
	for (int i = 0; i < 100; ++i) {
		std::cout << "canConnect:" << canConnect << std::endl;


		//GamePacket* packet = nullptr;
		//serverCat.WriteFullPacket(&packet);  // 可以改为 WriteDeltaPacket 来发送增量状态
		//server->SendGlobalPacket(*packet);
		//delete packet;

		// 服务器发送全局消息给所有客户端
		StringPacket s1 = StringPacket("Server says hello! " + std::to_string(i));
		server->SendGlobalPacket(s1);

		// 客户端发送消息给服务器
		StringPacket s2 = StringPacket("Client says hello! " + std::to_string(i));
		client->SendPacket(s2);

		// 更新服务器和客户端状态
		server->UpdateServer();
		client->UpdateClient();

		// 稍微等待一段时间再发送下一个消息，避免占用过多资源
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// 清理网络资源
	NetworkBase::Destroy();
}


// 初始化构建游戏世界
TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {

	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);
	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = true;
	cameraMain = &world->GetMainCamera();
	gameCurrentTime = gameTime;

	world->GetMainCamera().SetController(controller);
	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");
	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
	InitWorld();

}

void TutorialGame::InitCamera() {
	//menu1 = false;
	//menu2 = false;

	pauseMenu = false;
	mainMenuShow = false;

	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));

	//lockedObject = nullptr;
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	//Mesh
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	catMesh		= renderer->LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh	= renderer->LoadMesh("Kitten.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	basicTex	= renderer->LoadTexture("checkerboard.png");
	gooseMesh = renderer->LoadMesh("goose.msh");

	coinMesh = renderer->LoadMesh("coin.msh");
	goat = renderer->LoadMesh("goat.msh");

	// doge picture
	DefaultTex = renderer->LoadTexture("Default.png");
	keyTex = renderer->LoadTexture("key.png");


	//shader
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	//InitCamera();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete catMesh;
	delete kittenMesh;
	delete enemyMesh;
	delete bonusMesh;
	delete capsuleMesh;
	delete basicTex;
	delete DefaultTex;
	delete basicShader;
	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (mainMenuShow) {
		displayMainMenu(dt);
		return;
	}
	//if (menu2) {
	//	Debug::Print("Welcome to the Main Menu!", Vector2(30, 30));
	//	Debug::Print("Start Game ", Vector2(30, 40));
	//	Debug::Print(">Exit Game", Vector2(30, 50));
	//	physics->Update(dt);
	//	//world->UpdateWorld(dt);
	//	renderer->Update(dt);
	//	renderer->Render();
	//	Debug::UpdateRenderables(dt);
	//	return;
	//}
	if (pauseMenu) {
		Debug::Print("Game Stoping, Press return back to the game", Vector2(50, 50));
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
		return;
	}
	//if (!inSelectionMode) {
	//	world->GetMainCamera().UpdateCamera(dt);
	//}
	//if (lockedObject != nullptr) {
	//	Vector3 objPos = lockedObject->GetTransform().GetPosition();
	//	Vector3 camPos = objPos + lockedOffset;
	//	float deltaX = Window::GetMouse()->GetRelativePosition().x;
	//	float deltaY = Window::GetMouse()->GetRelativePosition().y;
	//	//std::cout << "Y" << deltaY << "\n";
	//	//std::cout << "X" << deltaX << "\n";
	//	Matrix4 temp = Matrix::View(camPos, objPos, Vector3(0,1,0));
	//	Matrix4 modelMat = Matrix::Inverse(temp);
	//	Quaternion q(modelMat);
	//	Vector3 angles = q.ToEuler(); //nearly there now!
	//	world->GetMainCamera().SetPosition(camPos);
	//	world->GetMainCamera().SetPitch(angles.x);
	//	world->GetMainCamera().SetYaw(angles.y);
	//}
	//if (lockedObject != nullptr) {
	//	Vector3 objPos = lockedObject->GetTransform().GetPosition();
	//	// 根据鼠标的相对移动调整相机的水平偏移角度
	//	float deltaX = Window::GetMouse()->GetRelativePosition().x;
	//	float rotationSpeed = 1.0f; // 控制旋转速度
	//	float yawAngle = deltaX * rotationSpeed;
	//	// 使用四元数绕Y轴旋转偏移向量
	//	Quaternion rotation = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), yawAngle);
	//	lockedOffset = rotation * lockedOffset;
	//	// 更新相机位置
	//	Vector3 camPos = objPos + lockedOffset;
	//	// 设置相机始终朝向锁定物体
	//	world->GetMainCamera().SetPosition(camPos);
	//	world->GetMainCamera().SetYaw(atan2(-lockedOffset.x, -lockedOffset.z) * (180.0f / PI));
	//	world->GetMainCamera().SetPitch(0.0f); // 暂时固定俯仰角
	//}
	// target 是角色（玩家）的 GameObject
	if (lockedObject != nullptr) { 
		Vector3 targetPos = lockedObject->GetTransform().GetPosition();

		// 获取鼠标偏移量
		float deltaX = Window::GetMouse()->GetRelativePosition().x;
		float deltaY = Window::GetMouse()->GetRelativePosition().y;

		// 更新相机的 Yaw 和 Pitch
		static float yaw = 180.0f;    // 水平旋转角度
		static float pitch = -30.0f; // 垂直旋转角度，初始仰角
		const float rotationSpeed = 5.0f;  // 控制旋转速度
		const float pitchMin = -45.0f;    // 垂直最小角度
		const float pitchMax = 45.0f;     // 垂直最大角度

		yaw -= deltaX * rotationSpeed;   // 鼠标左移为正，镜头右转
		pitch -= deltaY * rotationSpeed; // 鼠标上移为正，镜头向下看

		// 限制 Pitch 角度范围
		pitch = std::clamp(pitch, pitchMin, pitchMax);

		// 角度转弧度公式
		auto DegToRad = [](float degrees) {
			return degrees * (3.14159265359f / 180.0f);
			};

		// 将角度转换为弧度
		float yawRad = -DegToRad(yaw);
		float pitchRad = DegToRad(pitch);

		// 计算四元数的旋转
		Quaternion pitchQuat = Quaternion(cos(pitchRad * 0.5f), sin(pitchRad * 0.5f), 0.0f, 0.0f);
		Quaternion yawQuat = Quaternion(cos(yawRad * 0.5f), 0.0f, sin(yawRad * 0.5f), 0.0f);

		// 合并 Pitch 和 Yaw 的旋转
		Quaternion rotation = yawQuat * pitchQuat;

		// 计算相机位置偏移
		Vector3 offset = yawQuat * Vector3(0, -5.0, -10.0f); // 相机距离固定为 10

		// 更新相机位置和方向
		Vector3 cameraPos = targetPos + offset;
		world->GetMainCamera().SetPosition(cameraPos);
		world->GetMainCamera().SetYaw(yaw);
		world->GetMainCamera().SetPitch(pitch);
	}

	if (server && isServer) {
		Debug::Print("Server", Vector2(5, 5));
		server->UpdateServer();

	}
	if (client && isClient)
	{
		Debug::Print("Client", Vector2(5, 5));
		client->UpdateClient();
	}

	physics->UseGravity(useGravity);
	UpdateGameTree(dt);
	UpdateKeys();
	//if (EnemyObject != nullptr) EnemyObject->Update(dt);
	if (enemyObject != nullptr) {
	
		enemyObject->GetStateMachine()->Update(dt);
		//std::cout << enemyObject->GetStateMachine()->GetActiveState();

	}
	if (keeperObject != nullptr) {
		keeperObject->GetStateMachine2()->Update(dt);
	}
	if (FriendObject != nullptr)
	{
		UpdateFriendMovement(FriendObject,dt);
	}
	//if (useGravity) {
	//	Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	//}
	//else {
	//	Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	//}

	//This year we can draw debug textures as well!

	//press K to release cat rayer!!!!
	RayCollision closestCollision;
	if (Window::GetMouse()->ButtonHeld(NCL::MouseButtons::Left) && selectionObject && superPowerActive) {

		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest && objClosest->GetRenderObject()) {
				//objClosest->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;
			Debug::DrawLine(rayPos, objClosest->GetTransform().GetPosition(), Vector4(1, 1, 1, 1));

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));

			selectionObject->GetPhysicsObject()->AddForce(rayDir*1000.0f);

		}
	}

	// Gaming State!!!!!!!
	if (!gameWin && !gameLose) {
		gameCurrentTime -= dt;
		int minute = floor(gameCurrentTime / 60.0f);
		int second = std::round(gameCurrentTime - minute * 60.0f);
		if (second == 60) {
			second = 0;
			minute++;
		}
		std::string timeText = "Remaining time: "+std::to_string(minute)+":"+std::to_string(second);
		Vector4 timeColor = gameCurrentTime <= 30.0f ? Debug::RED : Debug::WHITE;
		Debug::Print(timeText, Vector2(5, 10), timeColor);
		std::string kittenText = "Kittens Got: " + std::to_string(kittenGotNum);
		Debug::Print(kittenText, Vector2(5, 15), Debug::WHITE);

		std::string kittenText2 = "Kittens Left: " + std::to_string(kittenTotalNum - kittenGotNum);
		Debug::Print(kittenText2, Vector2(5, 20), Debug::WHITE);

		if (hasKey) {
			Debug::DrawTex(*keyTex, Vector2(90, 10), Vector2(5, 5));
		}

	}

	if (gameCurrentTime <= 0.0f) {
		gameCurrentTime = 0.0f;
		gameLose = true;
	}

	if (PlayerObject) { // 确保 PlayerObject 存在
		float playerY = PlayerObject->GetTransform().GetPosition().y;
		if (playerY < -30.0f) {
			gameLose = true; // 设置游戏失败状态
			std::cout << "Player fell below Y=10. Game Over!\n";
		}
	}


	if (fabs((enemyObject->GetTransform().GetPosition() - PlayerObject->GetTransform().GetPosition()).Length()) < 3.0f)
		gameLose = true;

	if (gameWin || gameLose) {
		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);
		world->GetMainCamera().SetPosition(Vector3(50, 50, 0));
		if (gameWin) {
			std::string tit = "Congratulations! You win!";
			Debug::Print(tit, Vector2(30, 40), Debug::GREEN);

		}
		else {
			std::string tit = "You Lose!!! ";
			Debug::Print(tit, Vector2(30, 40), Debug::RED);
		}

		gameScore = gameCurrentTime + kittenGotNum * 1000.0f;
		std::string score = "Score: " + std::to_string(gameScore);;
		Debug::Print(score, Vector2(30, 50));

		std::string text = "Play Again(F1);";

		Debug::Print(text, Vector2(30, 70));

		text = "Exit(ESC);";
		Debug::Print(text, Vector2(30, 80));
		renderer->Render();
		Debug::UpdateRenderables(dt);
		return;
	}


	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 1, 1, 1));
	Debug::DrawLine(Vector3(), Vector3(100, 0, 0), Vector4(1, 1, 1, 1));
	Debug::DrawLine(Vector3(), Vector3(0, 0, 100), Vector4(1, 1, 1, 1));

	//if (testStateObject) {
	//	testStateObject -> Update(dt);
	//}
	for (int i = 1; i < pathToFollow.size(); ++i) {
		Vector3 a = pathToFollow[i - 1];
		Vector3 b = pathToFollow[i];
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
	for (int i = 1; i < pathToFollow2.size(); ++i) {
		Vector3 a = pathToFollow2[i - 1];
		Vector3 b = pathToFollow2[i];
		Debug::DrawLine(a, b, Vector4(0, 0, 1, 1));
	}

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {


	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {

		if (client && isClient) {
			
		}

		
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F4)) {
		if (server && isServer) {
			std::cout << "Server is sending a message to the client!" << std::endl;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F5)) {
		//TestNetworking();
		AddFriend();
		if (isClient) {
			ScorePacket ss = ScorePacket(100);
			client->SendPacket(ss);
		}
		if (isServer) {
			ScorePacket ss = ScorePacket(100);
			server->SendGlobalPacket(ss);
		}

	}
	//if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
	//	useGravity = !useGravity; //Toggle gravity!
	//	physics->UseGravity(useGravity);
	//}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		//selectionObject = nullptr;
	}
}

void TutorialGame::RotateObjectTowards(GameObject* object, const Vector3& targetDirection) {
	PhysicsObject* physics = object->GetPhysicsObject();

	// 获取物体当前的面向方向
	Vector3 currentForward = object->GetTransform().GetOrientation() * Vector3(0, 0, 1); // 假设默认朝向-Z
	currentForward.y = 0.0f; // 忽略 Y 轴方向，保持平面旋转
	currentForward = Vector::Normalise(currentForward);

	// 计算目标方向
	Vector3 targetForward = Vector::Normalise(targetDirection);

	// 计算旋转轴
	Vector3 rotationAxis = Vector::Cross(currentForward, targetForward);

	// 计算旋转角度 (通过点积)
	float angle = acos(Vector::Dot(currentForward, targetForward));

	// 如果方向几乎一致，就不旋转
	if (angle < 1e-3f) {
		physics->SetAngularVelocity(Vector3(0, 0, 0)); // 停止角速度
		return;
	}

	// 计算旋转角度所需要的扭矩
	float maxTorque = 2.0f; // 扭矩最大值
	float torqueStrength = angle * 2.0f; // 调整旋转力度
	if (torqueStrength > maxTorque) {
		torqueStrength = maxTorque; // 限制旋转力度
	}

	// 施加旋转扭矩
	Vector3 torque = rotationAxis * torqueStrength;

	physics->AddTorque(torque);

	// 阻尼：如果旋转角度接近目标方向，逐渐停止
	if (angle < 0.1f) {
		physics->SetAngularVelocity(Vector3(0, 0, 0)); // 停止角速度
	}
}





void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld	= Matrix::Inverse(view);

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	Vector3 targetDirection = Vector3(0, 0, 0);
	// 初始速度
	Vector3 currentVelocity = selectionObject->GetPhysicsObject()->GetLinearVelocity();
	Vector3 targetVelocity = Vector3(0, currentVelocity.y, 0); // 保留y方向速度
	float moveSpeed = 15.0f; // 移动速度

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		//selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
		//selectionObject->GetPhysicsObject()->SetLinearVelocity(fwdAxis);
		targetVelocity += fwdAxis * moveSpeed;
		targetDirection += fwdAxis; // 目标方向更新
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		//selectionObject->GetPhysicsObject()->SetLinearVelocity(-fwdAxis);
		targetVelocity -= fwdAxis * moveSpeed;
		targetDirection += -fwdAxis;
	}

	// 左右移动
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		selectionObject->GetPhysicsObject()->SetLinearVelocity(-rightAxis); // 左移
		targetDirection -= rightAxis;
		targetVelocity -= rightAxis * moveSpeed;

	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		selectionObject->GetPhysicsObject()->SetLinearVelocity(rightAxis); // 右移
		targetDirection += rightAxis;
		targetVelocity += rightAxis * moveSpeed;

	}
	//跳跃
	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,3,0));
	}
	if (targetDirection.Length() > 0.0f) {
		RotateObjectTowards(selectionObject, targetDirection);

	}
	selectionObject->GetPhysicsObject()->SetLinearVelocity(targetVelocity);

}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			//selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}
		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			//selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));

		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
		}
		//跳跃
		if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 3, 0));
		}
	}
}



void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	objClosest = nullptr; // 清除指向已删除对象的指针
	selectionObject = nullptr;
	kittens.clear();
	kittenConstraints.clear();
	//InitCamera();
	//SelectObject();
	kittenGotNum = 0;
	gameLose = false;
	gameWin = false;
	superPowerActive = false;
	hasKey = false;
	pathToFollow.clear();
	pathToFollow2.clear();

	currentPathIndex = 0;
	currentPathIndex2 = 0;

	movingForward = true;
	//BridgeConstraintTest();
	InitFriendAIbehavior();
	InitMazeWorld();
	InitTimer();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);
	InitGameExamples();
	InitEnemyStateMachine(enemyObject, PlayerObject);
	InitKeeperStateMachine(keeperObject, PlayerObject,keyObject);

	
	//InitDefaultFloor();
	InitCollisionCallback();

}

void TutorialGame::InitMenu(int selectedIndex) {
	mainMenuShow = true;
	mainMenuIndex = selectedIndex;
}

void TutorialGame::PauseMenu() {
	pauseMenu = true;
};


/*

A single function to add a large immoveable cube to the bottom of our world
地板是AABB类型,200*200,厚度是2
*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();
	floor->SetName("floor");

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2.0f)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddHomeToWorld(const Vector3& position) {
	GameObject* home = new GameObject();
	home->SetName("home");

	Vector3 homeSize = Vector3(10, 1, 10);
	AABBVolume* volume = new AABBVolume(homeSize);
	home->SetBoundingVolume((CollisionVolume*)volume);
	home->GetTransform()
		.SetScale(homeSize * 1.0f)
		.SetPosition(position);

	home->SetRenderObject(new RenderObject(&home->GetTransform(), cubeMesh, DefaultTex, basicShader));
	home->SetPhysicsObject(new PhysicsObject(&home->GetTransform(), home->GetBoundingVolume()));

	home->GetPhysicsObject()->SetInverseMass(0);
	//home->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(home);

	return home;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/

//球体
//Sphere
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

//球体
//Sphere
GameObject* TutorialGame::AddKeyToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->SetName("key");

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);
	keyObject = sphere;

	return sphere;
}

//方块
//AABB类型
GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));

	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddDoorToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();
	cube->SetName("door");
	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));

	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));
	cube->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);
	DoorObject = cube;
	return cube;
}

//玩家大猫
//Sphere类型 2.0f的sphere
GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 2.0f;
	float inverseMass	= 9.0f;

	GameObject* character = new GameObject();
	character->SetName("Cat");

	SphereVolume* volume  = new SphereVolume(1.0f);
	//OBBVolume* volume = new OBBVolume(Vector3(10.0f, 1.0f, 10.0f));

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	selectionObject = character;
	PlayerObject = character;

	return character;
}

GameObject* TutorialGame::AddNetworkPlayerToWorld(const Vector3& position) {
	float meshSize = 2.0f;
	float inverseMass = 9.0f;

	GameObject* character = new GameObject();
	character->SetName("NetworkPlayer");

	SphereVolume* volume = new SphereVolume(1.0f);
	//OBBVolume* volume = new OBBVolume(Vector3(10.0f, 1.0f, 10.0f));

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), catMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	NetworkPlayerObject = character;

	return character;
}

GameObject* TutorialGame::AddFriendToWorld(const Vector3& position) {
	float meshSize = 2.0f;
	float inverseMass = 9.0f;

	GameObject* character = new GameObject();
	character->SetName("Goat");

	SphereVolume* volume = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), goat, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	FriendObject = character;

	return character;
}

//小小小猫
//Sphere类型 1.0f的sphere
GameObject* TutorialGame::AddKittenToWorld(const Vector3& position) {
	float meshSize = 1.0f;
	float inverseMass = 0.9f;

	GameObject* character = new GameObject();
	character->SetName("Kitten");
	SphereVolume* volume = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), kittenMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	character->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	kittens.push_back(character);
	return character;
}

//goose
GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;// 网格大小
	float inverseMass	= 0.9f;//反质量，0的时候无限大

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);
	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), gooseMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	character->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	enemyObject = character;
	NavigationPath path;
	Vector3 start = position;
	Vector3 end = Vector3(80, 0, 80);  // 假设终点是固定的
	if (grid->FindPath(start, end, path)) { // 假设 navigationGrid 是你的导航网格对象
		Vector3 waypoint;
		while (path.PopWaypoint(waypoint)) {
			pathToFollow.push_back(waypoint);
		}
	}



	return character;
}


GameObject* TutorialGame::AddKeeperToWorld(const Vector3& position) {
	float meshSize = 3.0f;// 网格大小
	float inverseMass = 0.9f;//反质量，0的时候无限大

	GameObject* character = new GameObject();
	character->SetName("keeper");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);
	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	character->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);
	keeperObject = character;
	return character;
}

//GameEnemyObject* TutorialGame::AddGameEnemyObject(const Vector3& position) {
//	float meshSize = 7.0f;
//	float inverseMass = 5.0f;
//
//	GameEnemyObject* character = new GameEnemyObject(grid, PlayerObject);
//
//	character->SetName("Enemy");
//	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
//	character->SetBoundingVolume((CollisionVolume*)volume);
//
//	character->GetTransform()
//		.SetScale(Vector3(meshSize, meshSize, meshSize))
//		.SetPosition(position);
//
//	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
//	character->GetRenderObject()->SetColour(Vector4(0.75, 0, 0, 1));
//	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
//
//	character->GetPhysicsObject()->SetInverseMass(inverseMass);
//	character->GetPhysicsObject()->InitSphereInertia();
//
//
//	world->AddGameObject(character);
//	//enemies.emplace_back(character);
//
//	return character;
//}

//猫头
GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* catHead = new GameObject();
	catHead->SetName("CatHead");

	SphereVolume* volume = new SphereVolume(0.5f);
	catHead->SetBoundingVolume((CollisionVolume*)volume);
	catHead->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	catHead->SetRenderObject(new RenderObject(&catHead->GetTransform(), bonusMesh, nullptr, basicShader));
	catHead->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	catHead->SetPhysicsObject(new PhysicsObject(&catHead->GetTransform(), catHead->GetBoundingVolume()));

	catHead->GetPhysicsObject()->SetInverseMass(1.0f);
	catHead->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(catHead);
	catBonus = catHead;
	return catHead;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), capsuleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}



void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, 0, 0));
	
}

void TutorialGame::InitGameExamples() {
	//AddKittenToWorld(Vector3(-20,3,-20));
	//AddKittenToWorld(Vector3(-30, 3, -30));
	//AddHomeToWorld(Vector3(-50, 3, 0));

	AddPlayerToWorld(Vector3(0, 0, 0));
	AddEnemyToWorld(Vector3(80, 0, 10));
	//AddBonusToWorld(Vector3(0, 25, 0));
	//AddPlayerToWorld(Vector3(0, 15, 0));
	//AddStateObjectToWorld(Vector3(0, 20, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	//if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
	//	inSelectionMode = !inSelectionMode;
	//	if (inSelectionMode) {
	//		Window::GetWindow()->ShowOSPointer(true);
	//		Window::GetWindow()->LockMouseToWindow(false);
	//	}
	//	else {
	//		Window::GetWindow()->ShowOSPointer(false);
	//		Window::GetWindow()->LockMouseToWindow(true);
	//	}
	//}
	if (inSelectionMode) {
		//Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));
		//selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

		//if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
		//	if (selectionObject) {	//set colour to deselected;
		//	selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		//	selectionObject = nullptr;
		//}

		//	Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
		//	RayCollision closestCollision;
		//if (world->Raycast(ray, closestCollision, true)) {
		//	selectionObject = (GameObject*)closestCollision.node;

		//	selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
		//		return true;
		//	}
		//	else {
		//		return false;
		//	}
		//}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)|| true) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	//Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	// 方块尺寸
	Vector3 cubeSize = Vector3(8, 8, 8);

	// 中间方块的质量倒数（越大越轻）
	float invCubeMass = 5;

	// 链接的数量
	int numLinks = 10;

	// 每个约束的最大距离
	float maxDistance = 30;

	// 每个方块之间的间距
	float cubeDistance = 20;

	// 起始位置
	Vector3 startPos = Vector3(0, 0, 0);

	// 创建桥的起始方块（质量为0，表示静止不动）
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	// 创建桥的终止方块（质量为0，表示静止不动）
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	// 设置当前链接的初始点为起始方块
	GameObject* previous = start;

	// 创建中间的桥链接
	for (int i = 0; i < numLinks; ++i) {
		// 在指定位置创建一个方块，具有指定的质量倒数
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);

		// 使用位置约束连接当前方块与上一个方块
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);

		// 更新当前方块为 "上一个方块"
		previous = block;
	}

	// 创建桥的最后一个约束，连接最后的中间方块与终止方块
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}


void TutorialGame::HandleCollision(GameObject* a, GameObject* b) {
	if ((a->GetName() == "Cat" && b->GetName() == "Kitten") ||
		(a->GetName() == "Kitten" && b->GetName() == "Cat")) {

		GameObject* cat = (a->GetName() == "Cat") ? a : b;
		GameObject* kitten = (a->GetName() == "Kitten") ? a : b;

		std::cout << "Cat has found a Kitten! Checking if Constraint already exists..." << std::endl;
		
		// 检查该小猫是否已经和大猫建立了约束
		if (kittenConstraints.find(kitten) == kittenConstraints.end()) {
			std::cout << "No existing constraint found. Creating a new one..." << std::endl;

			// 记录上一个小猫，初始时为空
			GameObject* lastKitten = nullptr;


			if (lastKitten == nullptr) {
				// 第一个小猫与大猫连接
				PositionConstraint* constraint = new PositionConstraint(cat, kitten, 3.0f);
				world->AddConstraint(constraint);
				
			}
			else {
				// 后续小猫与前一个小猫连接
				PositionConstraint* constraint = new PositionConstraint(lastKitten, kitten, 3.0f);
				world->AddConstraint(constraint);
			}

			// 记录小猫已经与大猫建立了约束
			kittenConstraints[kitten] = cat;
			kittenGotNum++;
			// 更新 lastKitten 为当前的小猫
			lastKitten = kitten;
		}
		else {
			std::cout << "Constraint already exists between Cat and Kitten!" << std::endl;
		}

		
	}
	// 获胜逻辑
	if ((a->GetName() == "Cat" && b->GetName() == "home") || (a->GetName() == "home" && b->GetName() == "Cat")) {	
		if (kittenGotNum == kittenTotalNum) {
			gameWin = true;
		}
	}
	// 超能力获取
	if ((a->GetName() == "Cat" && b->GetName() == "CatHead") || (a->GetName() == "CatHead" && b->GetName() == "Cat")) {
		superPowerActive = true;
		std::cout << "catHead pointer: " << catBonus << std::endl;
		if (catBonus != nullptr) {
			catBonus->SetActive(false);
		}
		else {
			std::cerr << "catHead is null!" << std::endl;
		}
	}
	// 钥匙获取
	if ((a->GetName() == "Cat" && b->GetName() == "key") || (a->GetName() == "key" && b->GetName() == "Cat")||
		(a->GetName() == "Goat" && b->GetName() == "key") || (a->GetName() == "key" && b->GetName() == "Goat")
		) {
		hasKey = true;
		if (keyObject != nullptr) {
			keyObject->SetActive(false);
		}
		else {
			std::cerr << "keyObject is null!" << std::endl;
		}
	}
	// 开门逻辑
	if ((a->GetName() == "Cat" && b->GetName() == "door") || (a->GetName() == "door" && b->GetName() == "Cat")) {
		if (DoorObject != nullptr && hasKey) {
			DoorObject->SetActive(false);
			AABBVolume* volume = new AABBVolume(Vector3(0,0,0));
			DoorObject->SetBoundingVolume((CollisionVolume*)volume);
		}
		else {
			std::cerr << "DoorObject is null!" << std::endl;
		}
	}
	// 农夫抢钥匙逻辑
	if ((a->GetName() == "Cat" && b->GetName() == "keeper") || (a->GetName() == "keeper" && b->GetName() == "Cat")) {
		if (hasKey) {
			keyObject->SetActive(true);
			hasKey = false;
			std::cerr << "Key lose" << std::endl;
		}
	}
}




void TutorialGame::InitCollisionCallback() {
	physics->SetCollisionCallback(
		[this](GameObject* a, GameObject* b) {
			HandleCollision(a, b);
		});
}


//初始化迷宫
void TutorialGame::InitMazeWorld() {

	if (grid == nullptr) {
		grid = new NavigationGrid("TestGrid1.txt");
	}

	int** gridSquare = grid->GetGrid();
	int size = grid->GetSize();

	Vector3 cubeDims = Vector3(1, 1, 1);

	// 生成地板
	AddCubeToWorld(Vector3((grid->GetWidth() * size) / 2.0f, -size / 2.0f, (grid->GetHeight() * size) / 2.0f),
		Vector3((grid->GetWidth() * size) / 2.0f,2.0f, (grid->GetHeight() * size) / 2.0f), 0.0f);

	for (int y = 0; y < grid->GetHeight(); y++) {
		for (int x = 0; x < grid->GetWidth(); x++) {
			if (gridSquare[y][x] == 120) {// x wall
				AddCubeToWorld(Vector3(x * size, 0, y * size), Vector3(size / 2, size , size / 2), 0.0f);
			}
			if (gridSquare[y][x] == 49) {//1 kitten
				AddKittenToWorld(Vector3(x * size, 0, y * size));
			}
			if (gridSquare[y][x] == 50) {//2 key
				AddKeyToWorld(Vector3(x * size, 0, y * size), 0.5, 0.0f);

			}
			if (gridSquare[y][x] == 51) {//3 bonus
				AddBonusToWorld(Vector3(x * size, 0, y * size));

			}
			if (gridSquare[y][x] == 52) {//4 home
				AddHomeToWorld(Vector3(x * size, 0, y * size));

			}
			if (gridSquare[y][x] == 53) {//5 door
				AddDoorToWorld(Vector3(x * size, 0, y * size), Vector3(size / 2, size, size / 2), 0.0f);
			}
			if (gridSquare[y][x] == 54) {//6 keeper
				AddKeeperToWorld(Vector3(x * size, 0, y * size));
			}
		}
	}

}


void TutorialGame::InitTimer() {
	gameCurrentTime = gameTime;
}

//展示主菜单
void TutorialGame::displayMainMenu(float dt) {
	Debug::Print("Welcome to CatCraft!", Vector2(30, 30));
	const char* menuOptions[] = {
		"Start Game",
		"Start As Client",
		"Start As Server",
		"Exit Game"
	};
	for (int i = 0; i < 4; ++i) {
		if (i == mainMenuIndex) {
			Debug::Print(">" + std::string(menuOptions[i]), Vector2(30, 40 + i * 10));
		}
		else {
			Debug::Print(menuOptions[i], Vector2(30, 40 + i * 10));
		}
	}
	//physics->Update(dt);
	//world->UpdateWorld(dt);
	//renderer->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);
	return;
}

void TutorialGame::UpdateEnemyMovement(GameObject* enemy, float dt) {
	if (pathToFollow.empty()) return;

	Vector3 currentPosition = enemy->GetTransform().GetPosition();
	Vector3 targetPosition = pathToFollow[currentPathIndex];

	// 计算方向并设置速度
	Vector3 direction = (targetPosition - currentPosition);
	direction = Vector::Normalise(direction);
	float speed = 10.0f;  // 设置速度
	enemy->GetPhysicsObject()->SetLinearVelocity(direction * speed);

	// 如果接近目标点，则切换到下一个路径点
	if ((targetPosition - currentPosition).Length() <= 1.0f) {
		if (movingForward) {
			currentPathIndex++;
			if (currentPathIndex >= pathToFollow.size()) {
				movingForward = false;  // 到达终点，切换为向后移动
				currentPathIndex = pathToFollow.size() - 2;
			}
		}
		else {
			currentPathIndex--;
			if (currentPathIndex < 0) {
				movingForward = true;  // 回到起点，切换为向前移动
				currentPathIndex = 1;
			}
		}
	}
}

void TutorialGame::UpdateFriendMovement(GameObject* goat, float dt) {
	if (pathToFollow2.empty()) return;

	Vector3 currentPosition = goat->GetTransform().GetPosition();
	Vector3 targetPosition = pathToFollow2[currentPathIndex2];

	// 计算方向并设置速度
	Vector3 direction = (targetPosition - currentPosition);
	direction = Vector::Normalise(direction);
	float speed = 5.0f;  // 设置速度
	float distance = direction.Length();
	if (distance < 0.5f) { // 如果已接近目标点，切换到下一个路径点
		currentPathIndex2++;
	}
	goat->GetPhysicsObject()->SetLinearVelocity(direction * speed);

}

void TutorialGame::InitEnemyStateMachine(GameObject* enemy, GameObject* player) {
	// 创建状态机
	StateMachine* enemyStateMachine = new StateMachine();

	// 巡逻状态
	State* patrolState = new State([enemy, this](float dt) {
		//enemy->MoveToTarget(dt); // 巡逻逻辑
		enemy->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1)); // 红色

		UpdateEnemyMovement(enemyObject, dt);
		});

	// 追击状态
	State* chaseState = new State([enemy, player](float dt) {
		Vector3 playerPos = player->GetTransform().GetPosition();
		Vector3 enemyPos = enemy->GetTransform().GetPosition();
		Vector3 dir = (playerPos - enemyPos);
		dir = Vector::Normalise(dir);

		enemy->GetPhysicsObject()->SetLinearVelocity(dir * 10.0f); // 朝玩家方向移动
		enemy->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1)); // 红色
		std::cout << "see you \n";
		});

	// 从巡逻到追击
	StateTransition* patrolToChase = new StateTransition(patrolState, chaseState, [enemy, player, this]() -> bool {
		Vector3 playerPos = player->GetTransform().GetPosition();
		Vector3 enemyPos = enemy->GetTransform().GetPosition();

		// 距离条件
		float distance = (playerPos - enemyPos).Length();
		if (distance < 15.0f) {
			return true;
		}

		// 射线检测条件
		//Vector3 direction = (playerPos - enemyPos);
		//direction = Vector::Normalise(direction);
		//Ray ray(enemyPos, direction);
		//Debug::DrawLine(enemyPos, playerPos, Vector4(1, 1, 0, 1)); // 黄色表示射线

		//RayCollision closestCollision;
		//if (world->Raycast(ray, closestCollision, true, enemy)) {
		//	GameObject* hitObject = (GameObject*)closestCollision.node;
		//	if (hitObject == player) {
		//		std::cout << "Player detected by ray!" << std::endl;
		//		return true; // 检测到玩家
		//	}
		//}

		//return false; // 不满足条件
		});

	// 从追击到巡逻
	StateTransition* chaseToPatrol = new StateTransition(chaseState, patrolState, [enemy, player]() -> bool {
		float distance = (player->GetTransform().GetPosition() - enemy->GetTransform().GetPosition()).Length();
		return distance > 20.0f; // 玩家离开视野范围
		});

	// 添加状态和转换到状态机
	enemyStateMachine->AddState(patrolState);
	enemyStateMachine->AddState(chaseState);
	enemyStateMachine->AddTransition(patrolToChase);
	enemyStateMachine->AddTransition(chaseToPatrol);

	enemy->SetStateMachine(enemyStateMachine); // 将状态机绑定到敌人
}

void TutorialGame::InitKeeperStateMachine(GameObject* keeper, GameObject* player, GameObject* keyObject) {
	// 创建状态机
	StateMachine* keeperStateMachine = new StateMachine();

	// Keeper 初始位置，用于追踪后返回原地
	Vector3 initialPosition = keeper->GetTransform().GetPosition();

	// 巡逻状态（原地保持）
	State* idleState = new State([keeper](float dt) {
		keeper->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0)); // 停止移动
		keeper->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));       // 蓝色表示原地状态
		});

	// 追逐状态
	State* chaseState = new State([keeper, player, keyObject, this, &initialPosition](float dt) {
		Vector3 playerPos = player->GetTransform().GetPosition();
		Vector3 keeperPos = keeper->GetTransform().GetPosition();
		//Vector3 dir = Vector::Normalise(playerPos - keeperPos);
		Vector3 dir = Vector::Normalise(Vector3(playerPos.x - keeperPos.x, 0, playerPos.z - keeperPos.z));
		keeper->GetPhysicsObject()->SetLinearVelocity(dir * 10.0f); // 朝玩家方向移动
		keeper->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1)); // 红色表示追逐状态

		// 绘制调试射线
		Debug::DrawLine(keeperPos, playerPos, Vector4(1, 0, 1, 1)); // 紫色射线
		});

	// 从原地保持到追逐状态的转换
	StateTransition* idleToChase = new StateTransition(idleState, chaseState, [this, keeper, player]() -> bool {
		if (!hasKey) {
			return false; // 玩家没有钥匙时，不进入追逐状态
		}

		// 射线检测
		Vector3 rayPos = keeper->GetTransform().GetPosition();
		Vector3 rayDir = Vector::Normalise(player->GetTransform().GetPosition() - rayPos);

		Ray ray(rayPos, rayDir);
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true, keeper)) {
			GameObject* hitObject = (GameObject*)closestCollision.node;
			if (hitObject == player) {
				std::cout << "Player detected by ray!" << std::endl;
				return true; // 检测到玩家，进入追逐状态
			}
		}

		// 绘制调试射线
		Debug::DrawLine(rayPos, rayPos + rayDir * 10.0f, Vector4(0, 1, 0, 1)); // 绿色射线
		return false; // 没有检测到玩家
		});

	// 从追逐到原地保持的转换
	StateTransition* chaseToIdle = new StateTransition(chaseState, idleState, [this, keeper, player]() -> bool {
		// 判断玩家没有钥匙或者玩家已经不在射线视线内
		if (!hasKey) {
			return true;  // 玩家没有钥匙，切换到原地保持状态
		}

		// 使用射线检测判断是否能看到玩家
		Vector3 rayPos = keeper->GetTransform().GetPosition();
		Vector3 rayDir = player->GetTransform().GetPosition() - rayPos;
		rayDir = Vector::Normalise(rayDir); // 计算朝向玩家的射线方向

		Ray ray(rayPos, rayDir);
		RayCollision closestCollision;

		// 射线检测是否有障碍物遮挡视线
		if (world->Raycast(ray, closestCollision, true, keeper)) {
			GameObject* hitObject = (GameObject*)closestCollision.node;
			if (hitObject == player) {
				return false; // 如果看到玩家，则继续追击
			}
		}

		// 如果看不到玩家，切换到原地保持状态
		return true;
		});


	// 添加状态和转换到状态机
	keeperStateMachine->AddState(idleState);
	keeperStateMachine->AddState(chaseState);
	keeperStateMachine->AddTransition(idleToChase);
	keeperStateMachine->AddTransition(chaseToIdle);

	// 绑定状态机到 Keeper
	keeper->SetStateMachine2(keeperStateMachine);
}

void TutorialGame::StartGame(int mode) {
	NetworkBase::Initialise();

	int port = NetworkBase::GetDefaultPort();

	// 创建测试包接收器，分别为服务器和客户端
	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	if (mode == 1) {
		// 客户端模式
		client = new GameClient();
		client->RegisterPacketHandler(String_Message, &clientReceiver);  // 客户端注册处理 String_Message 的回调

		std::cout << "Starting as Client..." << std::endl;

		isServer = false;
		isClient = true;
		// 初始化客户端
		std::cout << "port:" << port << std::endl;



		bool connected = client->Connect(127,0,0,1, port);
		if (connected) {
			std::cout << "Connected to server!" << std::endl;
			std::cout << "port:" << port << std::endl;
			ScorePacket ss = ScorePacket(100);
			client->SendPacket(ss);
		}
		else {
			std::cerr << "Failed to connect to the server!" << std::endl;
		}
	}
	else if (mode == 2) {
		// 服务器模式
		server = new GameServer(port, 1);
		std::cout << "Starting as Server..." << std::endl;
		std::cout << "port:" << port << std::endl;

		// 初始化服务器
		isServer = true;
		isClient = false;

		server->RegisterPacketHandler(String_Message, &serverReceiver);  // 服务器注册处理 String_Message 的回调
	}
}	

void TutorialGame::InitFriendAIbehavior() {
	// 动作节点：寻找钥匙
	BehaviourAction* findKey = new BehaviourAction("Find Key",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) { // 初始化状态
				Vector3 position = FriendObject->GetTransform().GetPosition(); // 玩家位置
				NavigationPath path;
				Vector3 start = position;
				Vector3 keyPosition = keyObject->GetTransform().GetPosition(); // 钥匙位置
				Vector3 end = keyPosition;  // 假设终点是固定的
				if (grid->FindPath(start, end, path)) { // 假设 navigationGrid 是你的导航网格对象
					Vector3 waypoint;
					while (path.PopWaypoint(waypoint)) {
						pathToFollow2.push_back(waypoint);
					}
				}
				return Ongoing; // 转为进行中状态
			}
			else if (state == Ongoing) { // 进行中状态
				UpdateFriendMovement(FriendObject, dt);
				if (!pathToFollow2.empty()) { // 倒计时结束
					std::cout << "Found a path!\n";
					return Success; // 寻找成功
				}
			}
			return state; // 保持进行中
		});

	// 动作节点：移动到房间
	BehaviourAction* goToDoor = new BehaviourAction("Go To Door",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) { // 初始化状态
				std::cout << "Going to the locked door!\n";
				return Ongoing; // 转为进行中状态
			}
			else if (state == Ongoing) { // 进行中状态

			}
			return state; // 保持进行中
		});

	// 动作节点：打开门
	BehaviourAction* openDoor = new BehaviourAction("Open Door",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) { // 初始化状态
				std::cout << "Opening Door!\n";
				return Success; // 动作直接成功
			}
			return state;
		});

	// 动作节点：寻找宝藏
	BehaviourAction* lookForBonus = new BehaviourAction("Look For Bonus",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) { // 初始化状态

				return Ongoing; // 转为进行中状态
			}
			else if (state == Ongoing) { // 进行中状态
				bool found = rand() % 2; // 随机判断是否找到宝藏
				if (found) {
					std::cout << "I found some treasure!\n";
					return Success; // 寻找成功
				}
				std::cout << "No treasure in here...\n";
				return Failure; // 寻找失败
			}
			return state;
		});

	// 动作节点：寻找物品
	BehaviourAction* FreezeEnmey = new BehaviourAction("Freeze!",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) { // 初始化状态

				return Ongoing; // 转为进行中状态
			}
			else if (state == Ongoing) { // 进行中状态
			enemyObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0)); // 停止敌人
			enemyObject->GetRenderObject()->SetColour(Vector4(0, 1, 1, 0.5)); // 红色表示冻结状态
				if (enemyObject->GetPhysicsObject()->GetLinearVelocity().Length() < 0.1f) {
					std::cout << "success\n";
					return Success; 
				}
				std::cout << "Failed \n";
				return Failure; 
			}
			return state;
		});

	// 在根节点先执行sequence，然后是selection，因为我是先addchild的sequence，所以先执行
	// sequence的逻辑是不能断开的，是序列，一失败就失败不执行下面的功能
	// selection的逻辑是可以选择的，lookForBonus失败了，才会执行FreezeEnmey
	sequence = new BehaviourSequence("Help open Door");
	sequence->AddChild(findKey);
	//sequence->AddChild(goToDoor);
	//sequence->AddChild(openDoor);

	selection = new BehaviourSelector("Help Cat");
	selection->AddChild(lookForBonus);
	selection->AddChild(FreezeEnmey);

	rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);//first
	//rootSequence->AddChild(selection);


	

	//std::cout << "We're going on an adventure!\n";
	//while (state == Ongoing) {
	//	state = rootSequence->Execute(1.0); // 模拟时间步长
	//}

	// 输出结果
	if (state == Success) {
		std::cout << "You're welcome!\n";
	}
	else if (state == Failure) {
		std::cout << "run away!\n";
	}

	std::cout << "All done!\n";
}

void TutorialGame::UpdateGameTree(float dt) {
	while (state == Ongoing) {
		state = rootSequence-> Execute(dt); // fake dt
	}
}

void TutorialGame::AddFriend() {
	AddFriendToWorld(Vector3(10, 0, 0)); // 增加一个友好角色 //FriendObject
	rootSequence->Reset(); // 重置行为树状态
	state = Ongoing;
}
#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "MenuStates.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

//vector <Vector3> testNodes;
//void TestPathfinding() {
//	NavigationGrid grid("TestGrid1.txt");
//	NavigationPath outPath;
//	Vector3 startPos(80, 0, 10);
//	Vector3 endPos(80, 0, 80);
//	bool found = grid.FindPath(startPos, endPos, outPath);
//	Vector3 pos;
//	while (outPath.PopWaypoint(pos)) {
//		testNodes.push_back(pos);
//	}
//}
//void DisplayPathfinding() {
//	for (int i = 1; i < testNodes.size(); ++i) {
//		Vector3 a = testNodes[i - 1];
//		Vector3 b = testNodes[i];
//		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
//	}
//}

//void TestStateMachine() {
//	StateMachine* testMachine = new StateMachine();
//	int data = 0;
//	State* A = new State([&](float dt)->void {
//		std::cout << "I'm in State A\n";
//		data++;
//		}
//	);
//	State* B = new State([&](float dt)->void {
//		std::cout << "I'm in State B\n";
//		data--;
//		}
//	);
//	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
//		return data > 10;
//		});
//	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
//		return data < 0;
//		});
//	testMachine->AddState(A);
//	testMachine->AddState(B);
//	testMachine->AddTransition(stateAB);
//	testMachine->AddTransition(stateBA);
//	for (int i = 0; i < 100; ++i) {
//		testMachine->Update(1.0f);
//	}
//}



//class PauseScreen : public PushdownState {
//public:
//    // 更新状态逻辑
//    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//        // 检查是否按下键盘上的 "U" 键
//        if (Window::GetKeyboard()->KeyPressed(KeyCodes::U)) {
//            return PushdownResult::Pop; // 弹出当前状态（返回到上一个状态）
//        }
//        return PushdownResult::NoChange; // 保持当前状态
//    }
//
//    // 当状态激活时调用
//    void OnAwake() override {
//        std::cout << "Press U to unpause game!\n"; // 提示玩家如何退出暂停
//    }
//};

//class GameScreen : public PushdownState {
//public:
//    // 更新状态逻辑
//    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//        // 减少暂停提醒计时器
//        pauseReminder -= dt;
//
//        // 每隔 1 秒显示当前的挖矿状态并提示用户操作
//        if (pauseReminder < 0) {
//            std::cout << "Coins mined: " << coinsMined << "\n";
//            std::cout << "Press P to pause game, or F1 to return to main menu!\n";
//            pauseReminder += 1.0f; // 重置计时器
//        }
//
//        // 检测是否按下 "P" 键，进入暂停状态
//        if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) {
//            *newState = new PauseScreen(); // 设置新状态为暂停屏幕
//            return PushdownResult::Push;  // 推入暂停状态
//        }
//
//        // 检测是否按下 "F1" 键，返回主菜单
//        if (Window::GetKeyboard()->KeyDown(KeyCodes::F1)) {
//            std::cout << "Returning to main menu!\n";
//            return PushdownResult::Pop; // 弹出当前状态，回到主菜单
//        }
//
//        // 模拟挖矿逻辑，每帧有 1/7 的概率增加金币数量
//        if (rand() % 7 == 0) {
//            coinsMined++;
//        }
//
//        return PushdownResult::NoChange; // 保持当前状态
//    };
//
//    // 当状态激活时调用
//    void OnAwake() override {
//        std::cout << "Preparing to mine coins!\n"; // 提示游戏开始
//    }
//
//protected:
//    int coinsMined = 0;       // 挖到的金币数量
//    float pauseReminder = 1;  // 暂停提示计时器（每秒触发一次）
//};

//class IntroScreen : public PushdownState {
//public:
//    // 更新状态逻辑
//    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//        // 检测用户是否按下 SPACE 键以进入游戏
//        if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
//            *newState = new GameScreen(); // 设置新状态为游戏屏幕
//            return PushdownResult::Push; // 推入游戏状态
//        }
//
//        // 检测用户是否按下 ESCAPE 键以退出
//        if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
//            return PushdownResult::Pop; // 弹出当前状态，返回上一级
//        }
//
//        return PushdownResult::NoChange; // 保持当前状态
//    }
//
//    // 当状态被激活时调用
//    void OnAwake() override {
//        // 输出欢迎信息和提示
//        std::cout << "Welcome to a really awesome game!\n";
//        std::cout << "Press Space To Begin or Escape to Quit!\n";
//    }
//};

//void TestPushdownAutomata(Window* w) {
//    // 创建推入式自动机，初始状态为 IntroScreen
//    PushdownMachine machine(new IntroScreen());
//
//    // 游戏主循环
//    while (w->UpdateWindow()) {
//        // 获取帧间隔时间 (dt) 用于状态更新
//        float dt = w->GetTimer().GetTimeDeltaSeconds();
//
//        // 更新推入式自动机，如果返回 false，退出循环
//        if (!machine.Update(dt)) {
//            return; // 结束程序
//        }
//    }
//}







/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
    //TestNetworking();
	WindowInitialisation initInfo;
	initInfo.width		= 1280;
	initInfo.height		= 720;
	initInfo.windowTitle = "CSC8503 Game technology!";

	Window*w = Window::CreateGameWindow(initInfo);

	if (!w->HasInitialised()) {
		return -1;
	}	

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

    //状态机
    //PushdownMachine machine(new MainMenuState());
    //TestPushdownAutomata(w);
    
	TutorialGame* g = new TutorialGame();
    PushdownMachine* menuState = new PushdownMachine(new MainMenuState(g));
	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	//TestPathfinding();
    //------------------------
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		//DisplayPathfinding();
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		//if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
		//	w->ShowConsole(true);
		//}
		//if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
		//	w->ShowConsole(false);
		//}

		//if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
		//	w->SetWindowPosition(0, 0);
		//}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
        //menuState->Update(dt);
       if (!menuState->Update(dt)) {
            break; // 退出程序
        }

	}

    //-------------------
    //    while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
    //    float dt = w->GetTimer().GetTimeDeltaSeconds();
    //    if (dt > 0.1f) {
    //        std::cout << "Skipping large time delta\n";
    //        continue;
    //    }
    //    w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
    //    // 更新推入式自动机
    //    if (!machine.Update(dt)) {
    //        break; // 退出程序
    //    }
    //}
	Window::DestroyGameWindow();
}
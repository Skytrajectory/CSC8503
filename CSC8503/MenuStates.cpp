

//#include "MenuStates.h"
//
//#include "PushdownMachine.h"
//#include "PushdownState.h"


//using namespace NCL;
//using namespace CSC8503;

//PushdownState::PushdownResult MenuState::OnUpdate(float dt, PushdownState** newState) {
//
//    if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM1)) {
//        *newState = new InitWorldState(g);
//        return PushdownResult::Push;
//    }
//    if (Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
//        return PushdownResult::Pop;
//    }
//    return PushdownResult::NoChange;
//}
//
//void MenuState::OnAwake() {
//    g->InitMenu();
//}

//PushdownState::PushdownResult InitWorldState::OnUpdate(float dt, PushdownState** newState) {
//    if (Window::GetKeyboard()->KeyDown(KeyCodes::F1))
//    {
//        g->InitWorld();
//        g->InitCamera();
//    }
//
//    if (Window::GetKeyboard()->KeyDown(KeyCodes::F3))
//        return PushdownResult::Pop;
//
//    if (Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
//        return PushdownResult::Pop;
//
//    return PushdownResult::NoChange;
//}

//void InitWorldState::OnAwake() {
//    g->InitWorld();
//    g->InitCamera();
//}
//#include "MenuStates.h"
//#include "PushdownMachine.h"
//#include "PushdownState.h"
//
//namespace NCL {
//    namespace CSC8503 {
//        // PauseMenuState
//        PauseMenuState::PauseMenuState(TutorialGame* g) : game(g) {}
//
//        PushdownResult PauseMenuState::OnUpdate(float dt, PushdownState** newState) {
//            DisplayMenu();
//
//            if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
//                if (selectedIndex == 0 || selectedIndex == 1) {
//                    return PushdownResult::Pop;
//                }
//            }
//
//            HandleInput();
//            return PushdownResult::NoChange;
//        }
//
//        void PauseMenuState::OnAwake() {
//            game->PauseMenu();
//            std::cout << "Game Stop!\n";
//        }
//
//        void PauseMenuState::DisplayMenu() {
//            // Placeholder for menu display logic
//        }
//
//        void PauseMenuState::HandleInput() {
//            if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP)) {
//                selectedIndex = (selectedIndex - 1 + 2) % 2;
//            }
//            if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
//                selectedIndex = (selectedIndex + 1) % 2;
//            }
//        }
//
//        // GamingState
//        GamingState::GamingState(TutorialGame* g) : game(g) {}
//
//        GamingState::~GamingState() {
//            delete game;
//        }
//
//        PushdownResult GamingState::OnUpdate(float dt, PushdownState** newState) {
//            if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) {
//                *newState = new PauseMenuState(game);
//                return PushdownResult::Push;
//            }
//            return PushdownResult::NoChange;
//        }
//
//        void GamingState::OnAwake() {
//            game->InitCamera();
//            std::cout << "Gaming\n";
//        }
//
//        // InitWorldState
//        InitWorldState::InitWorldState(TutorialGame* g) : game(g) {}
//
//        InitWorldState::~InitWorldState() {
//            delete game;
//        }
//
//        PushdownResult InitWorldState::OnUpdate(float dt, PushdownState** newState) {
//            *newState = new GamingState(game);
//            return PushdownResult::Push;
//        }
//
//        void InitWorldState::OnAwake() {
//            game->InitCamera();
//            game->InitTimer();
//            std::cout << "Game Started! Now is gaming.\n";
//        }
//
//        // MainMenuState
//        MainMenuState::MainMenuState(TutorialGame* g) : game(g) {}
//
//        PushdownResult MainMenuState::OnUpdate(float dt, PushdownState** newState) {
//            if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
//                *newState = new InitWorldState(game);
//                return PushdownResult::Push;
//            }
//
//            HandleInput();
//            game->InitMenu(selectedIndex);
//
//            return PushdownResult::NoChange;
//        }
//
//        void MainMenuState::OnAwake() {
//            std::cout << "MainMenuState On Awake\n";
//        }
//
//        void MainMenuState::HandleInput() {
//            if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP)) {
//                selectedIndex = (selectedIndex - 1 + 4) % 4;
//            }
//            if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
//                selectedIndex = (selectedIndex + 1) % 4;
//            }
//        }
//    }
//}
//

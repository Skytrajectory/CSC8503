#pragma once
#pragma once

#include "TutorialGame.h"
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "Window.h"
#include <iostream>

namespace NCL {
    namespace CSC8503 {

        //class MenuState : public PushdownState {
        //public:
        //    MenuState(TutorialGame* g) { this->g = g; };

        //    PushdownResult OnUpdate(float dt, PushdownState** newState) override;

        //    void OnAwake()override;

        //protected:
        //    TutorialGame* g;
        //};

        //class InitWorldState : public PushdownState {
        //public:
        //    InitWorldState(TutorialGame* g) { this->g = g; };

        //    PushdownResult OnUpdate(float dt, PushdownState** newState) override;

        //    void OnAwake()override;

        //protected:
        //    TutorialGame* g;
        //};
        class PauseMenuState : public PushdownState {
        public:
            PauseMenuState(TutorialGame* g) {
                this->game = g;
            }
            PushdownResult OnUpdate(float dt, PushdownState** newState) override {
                // 显示暂停菜单

                DisplayMenu();

                if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
                    if (selectedIndex == 0) {
                        return PushdownResult::Pop; // 返回主菜单
                    }
                    if (selectedIndex == 1) {
                        return PushdownResult::Pop; // 返回主菜单

                    }
                }

                HandleInput(); // 处理用户输入
                return PushdownResult::NoChange;
            }

            void OnAwake() override {
                game->PauseMenu();
                std::cout << "Game Stop!\n";
            }

        private:
            int selectedIndex = 0;
            TutorialGame* game;
            void DisplayMenu() {

            }

            void HandleInput() {
                if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP)) {
                    selectedIndex = (selectedIndex - 1 + 2) % 2;
                }
                if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
                    selectedIndex = (selectedIndex + 1) % 2;
                }
            }
        };

        class GamingState : public PushdownState {
        public:
            GamingState(TutorialGame* g) {
                this->game = g;
            }

            ~GamingState() {
                delete game;
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override {
                if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) {
                    *newState = new PauseMenuState(game);
                    return PushdownResult::Push;
                }

                //if (Window::GetKeyboard()->KeyDown(KeyCodes::O)) {
                //    return PushdownResult::Pop;
                //}

                //game->UpdateGame(dt); // 更新游戏逻辑
                return PushdownResult::NoChange;
            }

            void OnAwake() override {
                //game->InitWorld();
                game->InitCamera();
                //game->InitTimer();
                std::cout << "Gaming\n";
            }

        private:
            TutorialGame* game;
        };


        class InitWorldState : public PushdownState {
        public:
            InitWorldState(TutorialGame* g) {
                this->game = g;
            }

            ~InitWorldState() {
                delete game;
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override {
                //if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) {
                //    *newState = new PauseMenuState(game);
                //    return PushdownResult::Push;
                //}
                *newState = new GamingState(game);
                return PushdownResult::Push;
                //game->UpdateGame(dt); // 更新游戏逻辑
                //return PushdownResult::NoChange;
            }

            void OnAwake() override {
                //game->InitWorld();
                game->InitCamera();
                game->InitTimer();
                std::cout << "Game Started!Now is gamingggggggggggggggggggggggg\n";
            }

        private:
            TutorialGame* game;
        };


        class MainMenuState : public PushdownState {
        public:
            MainMenuState(TutorialGame* g) { 
                this->game = g; 
            };
            PushdownResult OnUpdate(float dt, PushdownState** newState) override {
                if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
                    if (selectedIndex == 0) {
                        *newState = new InitWorldState(game); // 进入游戏
                        return PushdownResult::Push;
                    }
                    if (selectedIndex == 1) {
                        *newState = new InitWorldState(game); // 进入游戏
                        game->StartGame(1);
                        return PushdownResult::Push;
                    }
                    if (selectedIndex == 2) {
                        *newState = new InitWorldState(game); // 进入游戏
                        game->StartGame(2);
                        return PushdownResult::Push;
                    }
                    if (selectedIndex == 3) {
                        return PushdownResult::Pop; // 退出游戏
                    }
                }
                HandleInput(); // 处理用户输入
                
                // 持续展示菜单
                game->InitMenu(selectedIndex);

                return PushdownResult::NoChange;
            }

            void OnAwake() override {
                //game->InitMenu(selectedIndex);
                std::cout << "MainMenuState On Awake\n";
            }

        private:
            int selectedIndex = 0;
            TutorialGame* game;
            void HandleInput() {
                if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP)) {
                    selectedIndex = (selectedIndex - 1 + 4) % 4;
                }
                if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
                    selectedIndex = (selectedIndex + 1) % 4;
                }
            }
        };
    }
}



//#pragma once
//
//#include "playerObject.h"
//#include "PhysicsObject.h"
//
//GamePlayerObject::GamePlayerObject() {
//
//}
//GamePlayerObject::~GamePlayerObject() {
//
//}
//void GamePlayerObject::OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) {
//
//    //if (name == "player" && otherObject->GetName() == "coinTools") {
//    //    otherObject->SetIsActive(false);
//    //    otherObject->SetBoundingVolume(nullptr);
//    //    score += 10;
//    //    itemsHasGet++;
//    //    itemsLeft--;
//    //}
//    //if (name == "player" && otherObject->GetName() == "sphereTools") {
//    //    otherObject->SetIsActive(false);
//    //    otherObject->SetBoundingVolume(nullptr);
//    //    score += 10;
//    //    itemsHasGet++;
//    //    itemsLeft--;
//    //}
//    //if (name == "player" && otherObject->GetName() == "keyTools") {
//    //    // otherObject->SetIsActive(false);
//    //    // otherObject->SetBoundingVolume(nullptr);
//    //    keyNum++;
//    //}
//    //if (name == "player" && otherObject->GetName() == "Door" && keyNum) {
//    //    otherObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0, -10, 0));
//    //}
//    //if (name == "player" && otherObject->GetName() == "endPoint" && keyNum) {
//    //    win = true;
//    //}
//}
//
//void GamePlayerObject::OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) {
//
//}
//
//void GamePlayerObject::Update(float dt)
//{
//
//    //    Vector3 linearMovement;
//    //    if (Window::GetKeyboard()->KeyHeld(KeyCodes::W)) linearMovement.z = -1.0f;
//    //    if (Window::GetKeyboard()->KeyHeld(KeyCodes::S)) linearMovement.z = 1.0f;
//    //    if (Window::GetKeyboard()->KeyHeld(KeyCodes::A)) linearMovement.x = -1.0f;
//    //    if (Window::GetKeyboard()->KeyHeld(KeyCodes::D)) linearMovement.x = 1.0f;
//
//      //  if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) && isOnGround) physicsObject->AddForce(Vector3(0, 1, 0) * jumpForce * 50.0f);
//
//    //    if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
//    //        physicsObject->AddTorque(Vector3(0, 10, 0));
//    //    }
//    //
//    //    if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
//    //        selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
//    //    }
//
//}
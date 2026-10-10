#include "Components/Collider.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/Input.hpp"
#include "Components/Position.hpp"
#include "Components/ShapeComponent.hpp"
#include "Core/Application.hpp"
#include "Inputs/Controller.hpp"
#include "Inputs/InputManager.hpp"
#include "Plugins/CollisionPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include <iostream>
#include <raylib.h>

class PlayerMovementSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, const float dt) override {
        if (!registry.HasResource<Engine::InputManager>()) {
            return;
        }

        auto& inputManager = registry.GetResource<Engine::InputManager>();

        for (Engine::Entity const entity : m_Entities) {
            if (!registry.HasComponent<Engine::Position>(entity)) {
                continue;
            }

            int playerIdx = 1;
            if (registry.HasComponent<Engine::Input>(entity)) {
                const auto& input = registry.GetComponent<Engine::Input>(entity);
                if (!input.isActive) {
                    continue;
                }
                playerIdx = input.playerIdx;
            }

            auto* ctrl = inputManager.GetController(playerIdx);
            if (!ctrl) {
                continue;
            }

            auto& pos = registry.GetComponent<Engine::Position>(entity);
            constexpr float speed = 300.0f;

            if (ctrl->isActionActive("MOVE_RIGHT")) {
                pos.x += speed * dt;
            }
            if (ctrl->isActionActive("MOVE_LEFT")) {
                pos.x -= speed * dt;
            }
            if (ctrl->isActionActive("MOVE_DOWN")) {
                pos.y += speed * dt;
            }
            if (ctrl->isActionActive("MOVE_UP")) {
                pos.y -= speed * dt;
            }

            // Window border boundaries (1280x720)
            if (pos.x < 40.0f) {
                pos.x = 40.0f;
            }
            if (pos.x > 1240.0f) {
                pos.x = 1240.0f;
            }
            if (pos.y < 40.0f) {
                pos.y = 40.0f;
            }
            if (pos.y > 680.0f) {
                pos.y = 680.0f;
            }
        }
    }
};

class ColliderBorderRenderSystem : public Engine::System {
  public:
    void Render(Engine::Registry& registry) override {
        auto collisionSystem = registry.GetSystem<Engine::CollisionSystem>();
        const auto& activeContacts = collisionSystem->GetActiveContacts();

        try {
            auto colliders = registry.GetComponentArray<Engine::Collider>();
            for (const auto& [entity, col] : *colliders) {
                if (!col.enabled || !registry.HasComponent<Engine::Position>(entity) ||
                    !registry.HasComponent<Engine::ShapeComponent>(entity)) {
                    continue;
                }

                const auto& shape = registry.GetComponent<Engine::ShapeComponent>(entity);

                bool isColliding = false;
                for (const auto& pair : activeContacts) {
                    if (pair.first == entity || pair.second == entity) {
                        isColliding = true;
                        break;
                    }
                }

                const Color color = isColliding ? RED : col.isTrigger ? YELLOW : GREEN;
                shape.color = color;
            }
        } catch (const std::exception&) {
        }
    }
};

int main() {
    std::cout << "======================================\n";
    std::cout << "          Collisions Sandbox          \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Collisions Sandbox Demo";
    config.targetFPS = 60;
    ChangeDirectory(GetApplicationDirectory());

    Engine::Application app(config);

    // Initialize plugins
    Engine::PositionPlugin posPlugin;
    posPlugin.Init(app.GetRegistry());

    Engine::TextPlugin textPlugin;
    textPlugin.Init(app.GetRegistry());

    Engine::CollisionPlugin colPlugin;
    colPlugin.Init(app.GetRegistry());

    Engine::InputPlugin inputPlugin;
    inputPlugin.Init(app.GetRegistry());

    // Register InputManager controller and keybinds
    auto& inputManager = app.GetRegistry().GetResource<Engine::InputManager>();
    inputManager.AddPlayer(1, std::make_unique<Engine::Controller>());
    auto* p1 = inputManager.GetController(1);
    if (p1) {
        p1->addKeybind("MOVE_UP", {KEY_W, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_DOWN", {KEY_S, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_LEFT", {KEY_A, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_RIGHT", {KEY_D, 0, Engine::KEYBOARD});
    }

    // Register collider border rendering system before shape system
    app.GetRegistry().RegisterSystem<ColliderBorderRenderSystem>();

    Engine::ShapePlugin shapePlugin;
    shapePlugin.Init(app.GetRegistry());

    // Instruction HUD text
    const auto infoEntity = app.GetRegistry().CreateEntity();
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), infoEntity,
                                       "Move Player (Circle) with WASD | Green: Solid Triangle | "
                                       "Yellow: Trigger Rectangle | Red: Colliding",
                                       20.0f, Engine::Position{30.0f, 20.0f});

    // Solid Triangle Entity (Polygon with 3 sides)
    const auto solidEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(solidEntity, Engine::Position{550.0f, 350.0f});
    Engine::ShapeComponent solidShape;
    solidShape.type = Engine::ShapeType::POLYGON;
    solidShape.sides = 3;
    solidShape.radius = 70.0f;
    solidShape.filled = true;
    solidShape.color = GREEN;
    app.GetRegistry().AddComponent(solidEntity, solidShape);
    app.GetRegistry().AddComponent(solidEntity, Engine::Collider(true, false));

    Engine::CollisionEvents solidEvents;
    solidEvents.onCollisionEnter.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[COLLISION ENTER] Solid Triangle (Entity " << self
                      << ") entered contact with Entity " << other << "!\n";
        });
    solidEvents.onCollisionExit.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[COLLISION EXIT] Solid Triangle (Entity " << self
                      << ") separated from Entity " << other << "!\n";
        });
    app.GetRegistry().AddComponent(solidEntity, solidEvents);

    // Trigger Zone Rectangle Entity
    const auto triggerEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(triggerEntity, Engine::Position{850.0f, 280.0f});
    Engine::ShapeComponent triggerShape;
    triggerShape.type = Engine::ShapeType::RECTANGLE;
    triggerShape.width = 150.0f;
    triggerShape.height = 150.0f;
    triggerShape.filled = true;
    triggerShape.color = YELLOW;
    app.GetRegistry().AddComponent(triggerEntity, triggerShape);
    app.GetRegistry().AddComponent(triggerEntity, Engine::Collider(true, true)); // isTrigger = true

    Engine::CollisionEvents triggerEvents;
    triggerEvents.onTriggerEnter.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[TRIGGER ENTER] Entity " << other << " entered Trigger Zone (Entity "
                      << self << ")!\n";
        });
    triggerEvents.onTriggerExit.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[TRIGGER EXIT] Entity " << other << " left Trigger Zone (Entity " << self
                      << ")!\n";
        });
    app.GetRegistry().AddComponent(triggerEntity, triggerEvents);

    // Player Circle Entity that moves
    const auto playerEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(playerEntity, Engine::Position{200.0f, 350.0f});
    Engine::ShapeComponent playerShape;
    playerShape.type = Engine::ShapeType::CIRCLE;
    playerShape.radius = 40.0f;
    playerShape.filled = true;
    playerShape.color = GREEN;
    app.GetRegistry().AddComponent(playerEntity, playerShape);
    app.GetRegistry().AddComponent(playerEntity, Engine::Collider(true, false));
    Engine::InputPlugin::AttachToEntity(app.GetRegistry(), playerEntity, 1);

    Engine::CollisionEvents playerEvents;
    playerEvents.onCollisionEnter.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[PLAYER] Collision started with Entity " << other << "!\n";
        });
    playerEvents.onCollisionExit.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[PLAYER] Collision ended with Entity " << other << "!\n";
        });
    playerEvents.onTriggerEnter.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[PLAYER] Entered Trigger Entity " << other << "!\n";
        });
    playerEvents.onTriggerExit.emplace_back(
        [](const Engine::Entity self, const Engine::Entity other) {
            std::cout << "[PLAYER] Exited Trigger Entity " << other << "!\n";
        });
    app.GetRegistry().AddComponent(playerEntity, playerEvents);

    auto moveSys = app.GetRegistry().RegisterSystem<PlayerMovementSystem>();
    moveSys->m_Entities.insert(playerEntity);

    app.Run();

    return 0;
}
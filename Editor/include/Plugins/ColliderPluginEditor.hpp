/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** ColliderPluginEditor.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_COLLIDERPLUGINEDITOR_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_COLLIDERPLUGINEDITOR_HPP
#include "PluginEditor.hpp"

namespace Editor {

class ColliderPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;

    const char* GetIcon() const override;

    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;

    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;

    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;

    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;
};
} // namespace Editor
#endif // G_ING_500_BAR_5_1_RTYPE_1_COLLIDERPLUGINEDITOR_HPP
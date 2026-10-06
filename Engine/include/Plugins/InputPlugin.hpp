/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputPlugin.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUTPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_INPUTPLUGIN_HPP
#include "IPlugin.hpp"
#include "Systems/TextSystem.hpp"

namespace Engine {
class InputPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    static void AttachToEntity(Registry& registry, Entity entity, int idxPlayer);
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUTPLUGIN_HPP

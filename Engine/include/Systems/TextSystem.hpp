/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
#include "ECS/System.hpp"
#include "Components/Position.hpp"

namespace Engine {
    class TextSystem : public System {
    public:
        void Update(Registry&registry, float dt) override;
    };
} // Engine

#endif //G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP

/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** IController.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#include <vector>

#include "InputManager.hpp"

namespace Engine {
class IController {
    virtual ~IController();
    virtual void poll() = 0;
    virtual void setDefaultKeymap() = 0;

  protected:
    std::vector<ActionType> _actionQueue;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP

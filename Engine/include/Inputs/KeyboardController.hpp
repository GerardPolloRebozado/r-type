/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** KeyboardController.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_KEYBOARDCONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_KEYBOARDCONTROLLER_HPP
#include "IController.hpp"

namespace Engine {
class KeyboardController : public IController {
  public:
    KeyboardController();
    ~KeyboardController() override;
    void poll() override;
    void setDefaultKeymap() override;
    void Update() override;
    const std::vector<ActionType>& getActionQueue() override;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_KEYBOARDCONTROLLER_HPP

# Welcome to R-Type Engine

The R-Type Engine is a custom-built, modern Entity Component System (ECS) game engine tailored for high-performance 2D games. Built in C++23 with Raylib, it provides a robust architecture for handling game logic, rendering, networking, and audio.

## Philosophy

Inspired by data-driven design, the engine strictly separates **Data (Components)** from **Logic (Systems)**. This results in highly decoupled, cache-friendly, and maintainable code.

- **Modular**: Everything is a plugin.
- **Fast**: Minimal overhead, memory-contiguous component arrays.
- **Extensible**: The Editor natively supports live-editing any registered component.

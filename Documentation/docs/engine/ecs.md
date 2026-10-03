# ECS Architecture

The core of the R-Type Engine is its Entity Component System.

## Entities
An `Entity` is simply an ID (integer). It contains no logic or data of its own.

## Components
Components are pure data structures (PODs).
```cpp
struct Transform {
    float x, y;
};
```

## Systems
Systems contain logic. They iterate over entities that possess specific components and update them every frame.

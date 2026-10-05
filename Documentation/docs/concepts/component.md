# Components

In our ECS architecture, a **Component** is purely a container for data.

Unlike traditional Object-Oriented Programming where an Object has both state (variables) and behavior (methods), ECS strictly separates them. A Component *only* contains state. It has absolutely no logic, no `Update()` function.

## Creating a Component

Creating a component is as simple as defining a C++ `struct`.

```cpp
struct Health {
    int current;
    int max;
    bool isInvincible;
};
```

## Best Practices

1. **Keep it simple**: Avoid putting pointers or complex STL containers inside components if possible.
2. **No Methods**: Do not add getter, setter, or update methods.
3. **Granular**: It's better to have multiple small components (e.g. `Position` and `Velocity`) than one massive component. This allows Systems to only query the exact data they need.

# Editor Design Notes

*This is a rough brain-dump of how I think our Editor architecture should work.*

### Workflow
`RTypeEditor` opens a Raylib window and immediately initializes the ImGui context. (I think that ImGui depends on another GUI to draw stuff, it seems like it is a component lib?)

We instantiate our `Engine::Registry` in memory. 

The actual game isn't drawn directly to the screen. The Engine's render system draws to a `RenderTexture2D` (a framebuffer). We then pass this texture to an ImGui window.

- We loop through the Registry to show all entities.
- When an entity is selected, we query its components (`registry.GetComponent<Transform>()`).
- We map those values to `ImGui::DragFloat` sliders. Moving the slider directly modifies the component data in memory.

We serialize the entire Registry state into `scene.json`. (although maybe we have to find a more optimal intermediate representation)

the `R-Type` game executable just reads `scene.json` and runs the engine loop

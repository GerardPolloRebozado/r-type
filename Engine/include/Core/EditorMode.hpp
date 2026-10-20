#ifndef ENGINE_CORE_EDITORMODE_HPP_
#define ENGINE_CORE_EDITORMODE_HPP_

namespace Engine {

/**
 * @struct EditorMode
 * @brief Registry marker meaning "this registry is being edited, not played".
 *
 * The editor adds this resource; games never do. Scripting respects it:
 * ScriptSystem::Update and Signals::EmitSignal become no-ops while present,
 * so Lua files never run in preview (no spawned entities, no destroyed rows,
 * no error spam against the scene being edited). Simulation systems
 * (positions, collisions) keep ticking — only Lua execution is gated.
 */
struct EditorMode {};

} // namespace Engine

#endif // ENGINE_CORE_EDITORMODE_HPP_

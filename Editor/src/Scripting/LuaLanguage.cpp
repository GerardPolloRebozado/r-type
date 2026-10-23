#include "Scripting/LuaLanguage.hpp"

#include <imgui.h>
#include <utility>

namespace Editor {
namespace {

void AddIdentifier(TextEditor::LanguageDefinition& language, const char* name,
                   const char* declaration) {
    TextEditor::Identifier identifier;
    identifier.mLocation = TextEditor::Coordinates(0, 0);
    identifier.mDeclaration = declaration;
    language.mIdentifiers.insert_or_assign(name, std::move(identifier));
}

} // namespace

TextEditor::LanguageDefinition MakeLuaLanguageDefinition() {
    TextEditor::LanguageDefinition language = TextEditor::LanguageDefinition::Lua();
    language.mAutoIndentation = true;
    language.mKeywords.erase("");

    // Keep these declarations short: they are rendered in a tooltip when the
    // cursor hovers an identifier
    AddIdentifier(language, "Entity", "Entity: numeric entity id");
    AddIdentifier(language, "Entity.new", "Entity.new(id) -> entity");
    AddIdentifier(language, "Entity.create", "Entity.create() -> entity");
    AddIdentifier(language, "Entity.destroy", "Entity.destroy(entity)");
    AddIdentifier(language, "Entity.queueDestroy", "Entity.queueDestroy(entity)");
    AddIdentifier(language, "Entity.find", "Entity.find(name) -> entity or nil");
    AddIdentifier(language, "Entity.getName", "Entity.getName(entity) -> string");
    AddIdentifier(language, "Entity.setName", "Entity.setName(entity, name)");
    AddIdentifier(language, "Component", "Component: component queries and signals");
    AddIdentifier(language, "Component.has", "Component.has(entity, typeName) -> bool");
    AddIdentifier(language, "Component.remove", "Component.remove(entity, typeName)");
    AddIdentifier(language, "Component.entitiesWith",
                  "Component.entitiesWith(typeName) -> array of entities");
    AddIdentifier(language, "Component.connect",
                  "Component.connect(emitter, signal, target, method)");
    AddIdentifier(language, "Component.disconnect",
                  "Component.disconnect(emitter, signal, target, method)");
    AddIdentifier(language, "Transform", "Transform: position, rotation and scale");
    AddIdentifier(language, "Transform.getPosition", "Transform.getPosition(entity) -> {x, y, z}");
    AddIdentifier(language, "Transform.setPosition", "Transform.setPosition(entity, x, y, z?)");
    AddIdentifier(language, "Transform.translate", "Transform.translate(entity, dx, dy)");
    AddIdentifier(language, "Transform.getZIndex", "Transform.getZIndex(entity) -> int");
    AddIdentifier(language, "Transform.setZIndex", "Transform.setZIndex(entity, z)");
    AddIdentifier(language, "Transform.getRotation", "Transform.getRotation(entity) -> degrees");
    AddIdentifier(language, "Transform.setRotation", "Transform.setRotation(entity, degrees)");
    AddIdentifier(language, "Transform.rotate", "Transform.rotate(entity, deltaDegrees)");
    AddIdentifier(language, "Controller", "Controller: per-player input actions");
    AddIdentifier(language, "Controller.hasController",
                  "Controller.hasController(playerId?) -> bool");
    AddIdentifier(language, "Controller.isActionActive",
                  "Controller.isActionActive(actionName, playerId?) -> bool");
    AddIdentifier(language, "Controller.getActiveActions",
                  "Controller.getActiveActions(playerId?) -> array");
    AddIdentifier(language, "Input", "Input: raw keyboard and mouse state");
    AddIdentifier(language, "Input.isKeyDown", "Input.isKeyDown(key) -> bool");
    AddIdentifier(language, "Input.isKeyPressed", "Input.isKeyPressed(key) -> bool");
    AddIdentifier(language, "Input.isKeyReleased", "Input.isKeyReleased(key) -> bool");
    AddIdentifier(language, "Input.getMousePosition", "Input.getMousePosition() -> {x, y}");
    AddIdentifier(language, "Time", "Time: frame timing helpers");
    AddIdentifier(language, "Time.deltaTime", "Time.deltaTime() -> fallback delta seconds");
    AddIdentifier(language, "Time.time", "Time.time() -> seconds since start");
    AddIdentifier(language, "Scene", "Scene: scene operations");
    AddIdentifier(language, "Scene.load", "Scene.load(scenePath)");
    AddIdentifier(language, "log", "log: write to the engine console");
    AddIdentifier(language, "log.info", "log.info(message)");
    AddIdentifier(language, "log.error", "log.error(message)");
    AddIdentifier(language, "log.debug", "log.debug(message)");
    AddIdentifier(language, "Net", "Net: multiplayer replication helpers");
    AddIdentifier(language, "Layers", "Layers: named collision layers");
    AddIdentifier(language, "KEY", "KEY: keyboard key constants");

    return language;
}

TextEditor::Palette MakeLuaEditorPalette() {
    TextEditor::Palette palette{};

    const auto hex = [](const char* s) -> ImU32 {
        unsigned int r, g, b;
        sscanf(s + 1, "%02x%02x%02x", &r, &g, &b);
        return IM_COL32(r, g, b, 255);
    };

    const auto hexA = [](const char* s, unsigned char a) -> ImU32 {
        unsigned int r, g, b;
        sscanf(s + 1, "%02x%02x%02x", &r, &g, &b);
        return IM_COL32(r, g, b, a);
    };

    palette[(int)TextEditor::PaletteIndex::Default] = hex("#d8d8d8");      // Foreground
    palette[(int)TextEditor::PaletteIndex::Keyword] = hex("#96bbdc");      // Blue (palette 4)
    palette[(int)TextEditor::PaletteIndex::Number] = hex("#e0bb71");       // Yellow (palette 3)
    palette[(int)TextEditor::PaletteIndex::String] = hex("#719672");       // Green (palette 2)
    palette[(int)TextEditor::PaletteIndex::CharLiteral] = hex("#719672");  // Green
    palette[(int)TextEditor::PaletteIndex::Punctuation] = hex("#d8d8d8");  // Foreground
    palette[(int)TextEditor::PaletteIndex::Preprocessor] = hex("#dfbdbc"); // Magenta (palette 5)
    palette[(int)TextEditor::PaletteIndex::Identifier] = hex("#d8d8d8");   // Foreground
    palette[(int)TextEditor::PaletteIndex::KnownIdentifier] =
        hex("#badcfb"); // Bright blue (palette 12)
    palette[(int)TextEditor::PaletteIndex::PreprocIdentifier] = hex("#dfbdbc"); // Magenta
    palette[(int)TextEditor::PaletteIndex::Comment] = hex("#606060"); // Dim gray (palette 0)
    palette[(int)TextEditor::PaletteIndex::MultiLineComment] = hex("#606060");  // Dim gray
    palette[(int)TextEditor::PaletteIndex::Background] = hex("#171717");        // Dark background
    palette[(int)TextEditor::PaletteIndex::Cursor] = hex("#d8d8d8");            // Cursor color
    palette[(int)TextEditor::PaletteIndex::Selection] = hexA("#96bbdc", 100);   // Blue selection
    palette[(int)TextEditor::PaletteIndex::ErrorMarker] = hexA("#df9a98", 110); // Red error
    palette[(int)TextEditor::PaletteIndex::Breakpoint] = hexA("#df9a98", 180);  // Red breakpoint
    palette[(int)TextEditor::PaletteIndex::LineNumber] = hex("#757575"); // Mid gray (palette 8)
    palette[(int)TextEditor::PaletteIndex::CurrentLineFill] =
        hexA("#96bbdc", 20); // Subtle blue line highlight
    palette[(int)TextEditor::PaletteIndex::CurrentLineFillInactive] =
        hexA("#757575", 15);                                                       // Subtle gray
    palette[(int)TextEditor::PaletteIndex::CurrentLineEdge] = hexA("#606060", 85); // Subtle edge

    return palette;
}

} // namespace Editor

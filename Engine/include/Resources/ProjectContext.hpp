/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Project folder context + asset path resolution (Engine side).
**
** Games and the editor run with the executable's directory as CWD, while
** asset/script paths stored in scenes are project-relative
** (e.g. "scripts/test.lua"). This resolver turns them into loadable paths
** using the ProjectContext resource (registered by the editor and by
** SceneSerializer::Deserialize).
**
** Compatibility shim: the canonical implementation lives in
** Assets/ProjectPaths.hpp (ResolveAssetPath + ToProjectRelativePath).
** This header re-exports it so legacy `#include
** "Resources/ProjectContext.hpp"` / `ResolveProjectPath` call sites
** (e.g. SpriteSystem) keep compiling, including on Windows where both
** headers previously defined `Engine::ProjectContext` and clashed.
*/
#ifndef ENGINE_RESOURCES_PROJECTCONTEXT_HPP_
#define ENGINE_RESOURCES_PROJECTCONTEXT_HPP_

#include "Assets/ProjectPaths.hpp"

#endif // ENGINE_RESOURCES_PROJECTCONTEXT_HPP_

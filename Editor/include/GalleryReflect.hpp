/**
 * @file GalleryReflect.hpp
 * @brief Glaze field map for the Gallery project list.
 *
 * Include this header in every translation unit that reads or writes
 * @c projects.json as @ref Editor::ProjectData. Without it Glaze cannot
 * match the JSON keys and the Gallery list stays empty.
 */

#ifndef EDITOR_GALLERY_REFLECT_HPP_
#define EDITOR_GALLERY_REFLECT_HPP_

#include "Gallery.hpp"
#include <glaze/glaze.hpp>

/**
 * @brief Maps @ref Editor::ProjectData members to @c projects.json keys.
 */
template <> struct glz::meta<Editor::ProjectData> {
    static constexpr auto value = glz::object(
        "name", &Editor::ProjectData::name, "description", &Editor::ProjectData::description,
        "lastOpened", &Editor::ProjectData::lastOpened, "path", &Editor::ProjectData::path,
        "engineVersion", &Editor::ProjectData::engineVersion);
};

#endif // EDITOR_GALLERY_REFLECT_HPP_

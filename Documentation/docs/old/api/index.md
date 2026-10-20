# API reference

The API pages are generated from the public engine headers with Doxygen and rendered by
`mkdoxy`. They are a lookup tool, not a tutorial: signatures, members, and inheritance,
without the ordering rules that the guides explain.

---

## Start here

<div class="grid cards" markdown>

-   **Classes**

    ---

    Every class and struct in `Engine::`, grouped alphabetically. Best entry point when you
    know the type name.

    [:octicons-arrow-right-24: Class index](../../Engine/annotated.md)

-   **Files**

    ---

    Browse by header. Useful when you are already in an `#include` and want to know what
    else lives next to it.

    [:octicons-arrow-right-24: File index](../../Engine/files.md)

-   **Namespaces**

    ---

    `Engine`, `Engine::Multiplayer`, `Engine::Signals`, `Engine::CollisionLayers`, `glz`,
    and friends.

    [:octicons-arrow-right-24: Namespace index](../../Engine/namespaces.md)

-   **Class hierarchy**

    ---

    Inheritance trees, mainly useful for `IPlugin`, `System`, and `LayeredRenderSystem`.

    [:octicons-arrow-right-24: Hierarchy](../../Engine/hierarchy.md)

</div>

---

## How to read a generated page

| Section | Meaning |
|---|---|
| **Public Member Functions** | Callable API; the documented ones include parameter semantics |
| **Public Attributes** | Component fields — these are the ones the editor and serializer see |
| **Detailed Description** | Contract and caveats; read this before using a class |
| **Source file** | Link to the `_source` page with the full header |

Generated pages include private members and static helpers because the Doxyfile sets
`EXTRACT_PRIVATE` and `EXTRACT_STATIC`. Not everything shown is intended for game code.
If a symbol is undocumented, prefer the matching guide page.

---

## Guides vs. reference

Use the guide when you are deciding **what to do**:

- [ECS architecture](../engine/ecs.md) — registry, systems, resources
- [Plugins](../engine/plugins.md) — registration and ordering
- [Networking](../engine/networking.md) — roles, replication, authority
- [Collisions](../engine/collisions.md) — SAT pipeline and event flow

Use the API pages when you already know the symbol and need its exact signature.

---

## Regenerating

The documentation site builds the API at the same time as the rest of the pages:

```bash
cd Documentation
mkdocs build
```

Doxygen is configured in `Documentation/Doxyfile`; it reads `Engine/include` and
`Editor/include` and writes XML to `doxygen_output/`, which `mkdoxy` consumes. Run a
`--strict` build before committing documentation changes.

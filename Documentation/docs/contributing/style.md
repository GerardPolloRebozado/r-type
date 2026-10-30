# Style and review

## Formatting

- Run clang-format before committing. The repository's `.clang-format` is the source of
  truth.
- Keep lines reasonable; prefer clarity over cleverness.

## Naming

- Components: PascalCase structs (`Position`, `SpriteComponent`).
- Systems: PascalCase with `System` suffix (`SpriteSystem`).
- Plugins: PascalCase with `Plugin` suffix (`SpritePlugin`).
- Resources: PascalCase (`AssetManager`).
- Files match their primary type.

## Commits

Follow Conventional Commits: `<type>(<scope>): <summary>`. See `CONTRIBUTING.md`.

## Pull requests

- Keep changes focused.
- Add tests for new behaviour.
- Update documentation when you change public APIs.
- Verify `cmake --build build --target Tests` passes.

## See also

- [Setup](setup.md)
- [Testing](testing.md)
- [Architecture](architecture-engine.md)

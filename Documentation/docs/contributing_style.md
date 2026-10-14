# Contributing Code Style

## C++ Style

- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use `namespace Engine { ... }` for all engine code
- Doxygen all public classes, functions, and structs
- Use `#pragma once` for header guards (project standard)
- Line length: 100 characters maximum
- Braces on own style: `{` on same line as declaration, `}` on own line

## Lua Style

- Use [LuaFormatter](https://github.com/KorenSpecific/LuaFormatter) or similar
- Comment Lua bindings with `/** ... */` Doxygen-style comments
- Capture `sol::state_view` by value in lambdas, never by reference
- Use `sol::state` (not `sol::state_view`) when you need to store the state

## Commit Messages

Conventional Commits format:
```
feat: add new component system
fix: resolve collision system crash
docs: update lua api reference
refactor: move Plugin system to separate file
```

## Branch Workflow

1. Branch from `development`
2. Work in a feature branch: `feature/xxx` or `bugfix/xxx`
3. Submit PR against `development`
4. CI runs tests and docs build
4. Merge after approval
<tool_call>
<parameter=path>
/home/alex/epitech/year3/game-engine-development/r-type/Documentation/contributing_style.md
# Contributing to the R-Type Engine

Thank you for considering contributing to the R-Type Engine! This document outlines the process and standards for contributing.

## Code of Conduct

Please be respectful and constructive in all interactions. This project follows the [ETSII Code of Conduct](https://etsii..upm.es).

## How to Contribute

### Reporting Issues

1. Check the [issue tracker](https://github.com/yourorg/rtype/issues) for existing reports
2. Create a new issue with a clear title and description
3. Include reproduction steps, engine version, and relevant logs

### Developing Features

1. Fork the repository and create a new branch from `development`
2. Follow the [coding standards](#coding-standards)
3. Write tests for new functionality
4. Ensure all existing tests pass
5. Submit a Pull Request with a clear description

### Documentation

1. Add or update documentation in `Documentation/docs/`
2. Ensure mkdocs build succeeds: `mkdocs build`
3. Update the navigation if needed (`Documentation/mkdocs.yml`)

### Bug Fixes

1. Isolate the bug and create a minimal reproduction
2. Fix the issue in a new branch
3. Add or update tests if applicable
4. Document the fix in the changelog or docs

## Development Setup

### Prerequisites

- C++23 compatible compiler (GCC 13+, Clang 16+)
- CMake 3.28+
- Raylib 4.2+
- GLFW 3.3+
- Enet 1.2+
- Glaze for serialization

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourorg/rtype.git
cd rtype

# Initialize submodules (if any)
git submodule update --init --recursive

# Build the engine
cmake -B build -S .
cmake --build build -j$(nproc)

# Build the editor
cmake -B editor_build -S . -DEDITOR_BUILD=ON
cmake --build editor_build --target RTypeEditor -j$(nproc)
```

### Coding Standards

- Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with project-specific exceptions
- Use `namespace Engine { ... }` for all engine code
- Use Doxygen `@brief` and `@param` tags for all public headers
- Run `cmake --build build --target Engine -j$(nproc)` to verify compilation
- Ensure `mkdocs build` passes without errors

### Commit Messages

Use the [conventional commits](https://conventionalcommits.org/) format:
```
feat: add new component system
fix: resolve collision system crash
docs: update lua api reference
```

## Pull Request Process

1. Ensure your branch is up to date with `development`
2. Run all tests: `cmake --build build --target Test -j$(nproc)`
3. Lint your code if available
4. Submit the PR with a clear title and description
5. Address any reviewer feedback
5. Merge once all checks pass

## Branching Model

- `main`: Stable releases
- `development`: Integration branch for features
- Feature branches: `feature/xxx` or `bugfix/xxx`
- Hotfix branches: `hotfix/xxx` for production issues

## License

By contributing, you agree that your contributions are subject to the project's [MIT License](https://opensource.org/licenses/MIT).
# Testing

## Running the tests

```bash
cmake --build build --target Tests
./build/Tests
```

## Scaffold tests

```bash
cmake --build build --target ScaffoldRoundTrip
./build/ScaffoldRoundTrip
bash Tests/scaffold_integration.sh
```

`Tests/scaffold_integration.sh` exercises the CLI from a Gallery project folder and from
`Engine/`, then compiles the generated sources (including an `EDITOR_BUILD` game target).

For a safe walkthrough under `/tmp`:

```bash
bash Tests/scaffold_cli_tmp.sh
```

To exercise the real `Engine/CMakeLists.txt`:

```bash
bash Tests/manual_scaffold_cli.sh
```

## Adding tests

Tests live under `Tests/`. Add coverage for new components, systems, and serialization
changes. Prefer tests that exercise the real registry and serializer over mocked ones.

## See also

- [Setup](setup.md)
- [Style](style.md)

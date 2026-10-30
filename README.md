# Astra Studio

The IDE for astraOS apps, for Windows (it also builds on Linux).

- **New project** from the SDK templates (`app`, `tabs`, `list`).
- **Editor** for QML, JavaScript and JSON, with highlighting.
- **Live preview** that loads the app with astraOS's own components
  (`Astra.Controls`, `Astra.Animation`) built from the astraOS sources, so it
  looks as on the phone. System services (`Astra.Core`) are preview stand-ins.
- **Design check**: runs `astra check`, the SDK's design-language lint, and
  lists what to fix.
- **Build**: `astra build` makes the `.apkg` to install or publish on Market.
- The SDK documentation (`docs/sdk`, `docs/DESIGN.md`) is bundled.

## Download

Every push builds `AstraStudio-<version>-windows-x64.zip` (Actions → build →
artifacts); tagged versions (`v*`) are attached to a GitHub release. Unzip
and run `AstraStudio.exe`.

The SDK is written in Python: install [Python 3](https://www.python.org/)
(the `py` launcher or `python` on PATH) to check and build apps.

## Building

Astra Studio is built against a checkout of
[astra](https://github.com/tsryzenstech/astra) next to it:

```
GitHub/
  astra/          the astraOS sources
  astra-studio/   this repository
```

Requirements: Qt 6.5 or newer (Widgets, Qml, Quick, QuickWidgets, Svg; 6.8
is what CI uses), CMake 3.21 and a C++17 compiler (MSVC 2022 on Windows).

```
cmake -S astra-studio -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build
cmake --install build --prefix dist/AstraStudio
windeployqt --qmldir astra-studio/resources --qmldir astra/frameworks/qml/controls dist/AstraStudio/AstraStudio.exe
```

`-DASTRA_SOURCE=<path>` points to an astra checkout somewhere else. While
developing, `ASTRA_SDK=<astra>/sdk` makes Studio use the checkout's SDK.

### CI

`.github/workflows/build.yml` builds the Windows zip and checks the Linux
build on every push. While `tsryzenstech/astra` is private, add a repository
secret `ASTRA_REPO_TOKEN` (a token that can read it) so CI can check it out.

## Layout

| Path | |
| --- | --- |
| `src/` | the program (main window, editor, preview, SDK runner) |
| `preview/Astra/Core/` | preview stand-ins for astraOS's services |
| `resources/` | `PreviewHost.qml`, the icon |
| `cmake/AstraPreviewModules.cmake` | builds the QML modules the preview loads |

## License

Apache-2.0

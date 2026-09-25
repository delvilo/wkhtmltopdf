# wkhtmltopdf and wkhtmltoimage

Linux command-line tools and a C library for converting one HTML document to
PDF or an image using **standard Qt5/WebKit**. Qt5's offscreen platform is used
by default, so a display server is not required. An explicit `QT_QPA_PLATFORM`
is respected.

## Supported scope

- One HTML input per conversion, from a URL, local file, stdin or C API memory.
- PDF output with automatic pagination, paper size, margins, orientation,
  grayscale, print resolution and document title.
- Image output with smart/fixed width, automatic/fixed height, crop, quality,
  transparency, layout zoom and standard SVG serialization.
- Network loading, cookies, proxies, authentication, POST, stylesheets and
  JavaScript readiness controls.

Qt4, patched Qt and their build/submodule paths are removed. Multiple HTML
inputs, covers, document merging, page numbering, headers/footers, automatic
TOCs, PDF bookmarks, clickable PDF links and the SVG view-box clipping extension
are removed. Qt6/WebEngine is not selected. See
[removed options and migration](docs/removed-options.md) before reusing upstream
commands or library settings.

## Building on Linux

Qt5 5.3 or newer with WebKit, WebKitWidgets, SVG and PrintSupport is required.
The tested baseline and CI use Qt5 5.15. XML Patterns is no longer required.
For example, on Ubuntu 24.04:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake qtbase5-dev libqt5webkit5-dev libqt5svg5-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
build/bin/wkhtmltopdf input.html output.pdf
build/bin/wkhtmltoimage --format png --transparent input.html output.png
```

The CMake build creates `libwkhtmltox.so` alongside the executables in
`build/bin`. To install the library, C headers, tools and manpages, run
`cmake --install build --prefix /your/install/prefix`. Use
`-DWKHTMLTOX_VERSION=...` to override the default version shown by the tools.

Use the source build above for this fork. Upstream prebuilt packages use a
different feature set. The upstream website snapshots under `docs/` are retained
as historical references; this README, the migration guide and freshly generated
`--extended-help` describe the current fork.

## Architecture and checks

- [Rendering interfaces](docs/rendering-interfaces.md): network loading, DOM,
  whole-document printing and image painting boundaries, plus regression commands.
- [Image conversion](docs/image-entry-refactoring.md): validation, transactional
  file output and regression coverage.

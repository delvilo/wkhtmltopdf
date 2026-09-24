# Image entry refactoring

This change implements the selected items 1, 2, 4, 6 and 7:

| Item | Implementation |
| --- | --- |
| 1. Shared CLI startup | `CliApplication` owns locale/platform setup and the Qt style for both executables. Help and version parsing still run before constructing the application. Explicit `QT_QPA_PLATFORM` values remain respected. |
| 2. Smaller parser interface | Remove the unused image-parser `final` argument and the unused entry-point include. Make the shared argument array read-only throughout the parser so neither executable needs an `argv` cast. |
| 4. Validate image settings | Resolve the output format and check numeric settings before loading the input, including when called through the C API. Check the resulting crop and raster allocation after layout. |
| 6. Finish once | Separate rendering from conversion completion. Rendering, encoding and output errors return immediately and emit one failed completion; only successful output emits a successful completion. |
| 7. Complete file replacement | Stage regular-file output beside its destination, then replace the destination only after rendering, encoding and flushing succeed. |

## Validation and defaults

- Width must be positive. Height `0` retains automatic sizing; negative heights
  are rejected. Smart width remains enabled by default.
- Quality accepts `0..100` and `-1` for the Qt encoder default. The application
  default remains `94`.
- Crop offsets accept non-negative values and the existing `-1` default, which
  means zero. Crop dimensions accept positive values and `-1`, which means the
  remaining image. Zero dimensions and values below `-1` are rejected.
- Crop coordinates outside the rendered viewport fail. Large crop dimensions
  are clipped before rectangle construction to avoid integer overflow.
- Raster images must fit Qt 4's signed-int ARGB32 byte-count limit. Allocation
  failure is reported without overwriting the destination.
- Explicit formats and inferred file suffixes are normalized to lowercase.
  SVG and the formats reported by the installed Qt image writers are accepted.
  An unknown format, or a file without a usable suffix and explicit format,
  fails before loading the page.
- Stdout retains JPEG as its default. C API memory output now also defaults to
  JPEG when no format is provided; an explicit format still takes precedence.

## Output behavior

Qt 5.1 and later use `QSaveFile` with direct-write fallback disabled. Older Qt
versions use a sibling `QTemporaryFile` and a platform replacement operation.
Existing file permissions are preserved. On the older-Qt fallback, new files
retain the owner-only permissions of `QTemporaryFile`.

The destination directory must permit creating and replacing files. Existing
symbolic links are followed and retained. Non-regular output paths, such as a
directory or device, are rejected; use `-` to stream to stdout. Failed output
removes the temporary file and leaves an existing destination intact.

SVG is serialized into a memory buffer first because `QSvgGenerator` does not
propagate a short device write to the conversion result. The complete SVG write
is checked before committing the file. This uses additional memory proportional
to the SVG output size.

Stdout is checked for encoding and flush errors, but bytes already sent to a
stream cannot be rolled back. C API memory output is cleared on failure.

## Regression checks

After building the shared library and both executables, run with the library
available on the platform's library search path:

```sh
python3 tests/image_entry_smoke.py --bin-dir bin
python3 tests/option_removal_smoke.py --bin-dir bin
```

The image suite covers startup, validation, format inference, automatic height,
cropping, stdin/stdout, file replacement, symbolic links, C API memory output
and single completion callbacks. On POSIX it injects PNG and SVG write failures
using a file-size limit and checks that old contents survive without temporary
files. Where available, `/dev/full` checks stdout failures.

Validation performed on Linux with Qt 5.15 and unpatched QtWebKit: all 13 image
tests passed; the existing suite passed 6 tests and skipped its patched-Qt-only
test. Auto-sized, cropped and fixed-size PNG fixtures also matched their
pre-refactoring pixels exactly. Patched Qt 4 and Windows builds still require
validation in their respective environments.

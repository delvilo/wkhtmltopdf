---
layout: default
title: Removed options
---

# Removed options

This fork removes the following options from the command-line parser. Old names
are rejected as unknown arguments; they are not retained as compatibility aliases.

| Removed option | Replacement or fixed behavior |
| --- | --- |
| `--quiet`, `-q` | Use `--log-level none`. |
| `--default-header` | Use `--margin-top 2cm --header-left '[webpage]' --header-right '[page]/[topage]' --header-line`. Put the margin option before the first input. |
| `--readme`, `--htmldoc` | Use the checked-in documentation, `--help`, `--extended-help`, or `--manpage`. |
| `--lowquality`, `-l` | Printer initialization uses `QPrinter::HighResolution`. Control output explicitly with `--dpi`, `--image-dpi`, and `--image-quality`. |
| `--enable-plugins`, `--disable-plugins` | Browser plugins and Java are always disabled. JavaScript remains configurable. |
| `--use-xserver` | The CLI uses the existing default graphics mode: headless with patched Qt; Qt 5 Unix builds default to the offscreen platform. |
| `--copies`, `--collate`, `--no-collate` | Each document is rendered once. |
| `--no-pdf-compression` | PDF object compression is always enabled. |
| `--enable-forms`, `--disable-forms` | HTML form controls render as static page content, without interactive PDF fields. |
| `--checkbox-svg`, `--checkbox-checked-svg`, `--radiobutton-svg`, `--radiobutton-checked-svg` | Controls use their built-in appearance. |
| `--read-args-from-stdin` | Invoke the converter for each job. Input `-` still reads HTML from stdin; output `-` still writes to stdout. |
| `--dump-outline`, `--dump-default-toc-xsl` | Outline/stylesheet export is removed. PDF outlines, TOCs, and `--xsl-style-sheet` remain available with patched Qt. |

`--load-media-error-handling` accepts only `abort` and `ignore`; replace the removed
`skip` value with `ignore`. `--load-error-handling skip` still skips failed input
pages. These options deliberately use different accepted value sets.

## Library settings

The string setters/getters no longer expose these keys:

- PDF global: `quiet`, `useGraphics`, `resolution`, `copies`, `collate`,
  `dumpOutline`, `useCompression`.
- PDF object: `produceForms`, `web.enablePlugins`, `load.checkboxSvg`,
  `load.checkboxCheckedSvg`, `load.radiobuttonSvg`, `load.radiobuttonCheckedSvg`.
- Image global: `quiet`, `useGraphics`, `loadPage.checkboxSvg`,
  `loadPage.checkboxCheckedSvg`, `loadPage.radiobuttonSvg`,
  `loadPage.radiobuttonCheckedSvg`.

Check the return value from the C API's setting functions: removed keys return
failure. Use `logLevel=none` in place of `quiet=true` and `logLevel=info` in place
of `quiet=false`.

The C initialization functions retain their existing `use_graphics` argument.
The C++ settings layouts and customization signals changed, so rebuild C++
consumers against the new headers. Unused C++ arguments have also been removed: `object` from
`calculateHeaderHeight`, `fromStdin` from `parseArguments`, the SVG loader path,
the text outputter's `doc`/`extended` flags, `doc` from `outputSwitches`, and
`sure` from `outputNotPatched`. The unused `BookFunc`, HTML/README generators, and
commented-out image scale options have been removed too.

## Verification

After building the library and both executables, run:

```sh
python3 tests/option_removal_smoke.py --bin-dir bin
```

The smoke checks cover rejected options, retained error policies, stdin/stdout
conversion, PDF/image output, and C API settings. On patched Qt they also check
multi-object output, headers, TOCs, and the absence of interactive form fields.

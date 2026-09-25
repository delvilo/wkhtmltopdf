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
| `--default-header` | Headers, footers and page-number substitution are removed. |
| `--readme`, `--htmldoc` | Use the checked-in documentation, `--help`, `--extended-help`, or `--manpage`. |
| `--lowquality`, `-l` | Printer initialization uses `QPrinter::HighResolution`. Control print resolution with `--dpi`; PDF embedded-image controls are removed. |
| `--enable-plugins`, `--disable-plugins` | Browser plugins and Java are always disabled. JavaScript remains configurable. |
| `--use-xserver` | Qt5 defaults to the offscreen platform on Linux; an explicit `QT_QPA_PLATFORM` is respected. |
| `--copies`, `--collate`, `--no-collate` | Each document is rendered once. |
| `--no-pdf-compression` | PDF object compression is always enabled. |
| `--enable-forms`, `--disable-forms` | HTML form controls render as static page content, without interactive PDF fields. |
| `--checkbox-svg`, `--checkbox-checked-svg`, `--radiobutton-svg`, `--radiobutton-checked-svg` | Controls use their built-in appearance. |
| `--read-args-from-stdin` | Invoke the converter for each job. Input `-` still reads HTML from stdin; output `-` still writes to stdout. |
| `--dump-outline`, `--dump-default-toc-xsl` | Automatic TOCs, outlines/bookmarks and their exports are removed. |


## Standard Qt5 only

Qt4, patched Qt, the bundled Qt submodule and their CI/build paths are removed.
Use standard Qt5/WebKit on Linux. No Qt6/WebEngine migration is included.

The PDF command now accepts exactly one input:

```sh
wkhtmltopdf [OPTION]... input.html output.pdf
```

A single HTML document can still span many PDF pages. Multiple inputs and the
`page`, `cover`, `toc` object commands are rejected before loading or printing.
For files literally named `page`, `cover` or `toc`, use a path such as `./cover`.
Global options precede the input; page options can precede or follow it.

| Newly removed options | Current behavior |
| --- | --- |
| `--header-*`, `--footer-*`, `--no-header-line`, `--no-footer-line`, `--replace`, `--page-offset` | No generated headers, footers or page-number substitutions. |
| `--outline`, `--no-outline`, `--outline-depth`, `--include-in-outline`, `--exclude-from-outline` | No PDF bookmarks/outlines. |
| `--xsl-style-sheet`, `--toc-header-text`, `--disable-toc-links`, `--disable-dotted-lines`, `--toc-text-size-shrink`, `--toc-level-indentation`, `--enable-toc-back-links`, `--disable-toc-back-links` | No automatic TOC or XSLT processing; XML Patterns dependency removed. |
| `--enable-internal-links`, `--disable-internal-links`, `--enable-external-links`, `--disable-external-links`, `--resolve-relative-links`, `--keep-relative-links` | HTML links retain their visual content but do not generate clickable PDF links. |
| `--print-media-type`, `--no-print-media-type` | Standard Qt5 PDF printing uses print CSS; image rendering uses screen CSS. |
| `--enable-smart-shrinking`, `--disable-smart-shrinking` | Standard WebKit print shrinking remains automatic. Adjust HTML/CSS, paper/margins or `--zoom` for layout. |
| `--image-dpi`, `--image-quality` | PDF embedded-image encoding is handled by Qt5. Image-converter `--quality` is retained. |
| `--viewport-size` | Removed from PDF. Image-converter `--width` and `--height` are retained. |

Image `--enable-smart-width`, `--disable-smart-width` and `--transparent` are
retained and correctly advertised for Qt5. `--zoom` now applies standard Qt5
frame zoom; non-default values can change image/PDF layout compared with the
previous unpatched build, where the option was ignored. Ordinary SVG output is
retained; the patched `setViewBoxClip` extension is removed.

`--load-media-error-handling` accepts only `abort` and `ignore`; replace the removed
`skip` value with `ignore`. `--load-error-handling skip` still skips the failed input, but the
conversion fails because no printable document remains. These options deliberately use different accepted value sets.

## Library settings

The string setters/getters no longer expose these keys:

- PDF global: `quiet`, `useGraphics`, `resolution`, `copies`, `collate`,
  `dumpOutline`, `useCompression`, `resolveRelativeLinks`, `pageOffset`,
  `outline`, `outlineDepth`, `viewportSize`, `imageDPI`, `imageQuality`.
- PDF object: `produceForms`, `web.enablePlugins`, `load.checkboxSvg`,
  `load.checkboxCheckedSvg`, `load.radiobuttonSvg`, `load.radiobuttonCheckedSvg`,
  `header.*`, `footer.*`, `toc.*`, `tocXsl`, `isTableOfContent`, `includeInOutline`,
  `pagesCount`, `useExternalLinks`, `useLocalLinks`, `replacements`,
  `web.enableIntelligentShrinking`, `load.printMediaType`, `web.printMediaType`.
- Image global: `quiet`, `useGraphics`, `loadPage.checkboxSvg`,
  `loadPage.checkboxCheckedSvg`, `loadPage.radiobuttonSvg`,
  `loadPage.radiobuttonCheckedSvg`, `loadPage.printMediaType`,
  `web.enableIntelligentShrinking`.

Check the return value from the C API's setting functions: removed keys return
failure. Use `logLevel=none` in place of `quiet=true` and `logLevel=info` in place
of `quiet=false`.

The public C function signatures remain compatible. `wkhtmltopdf_extended_qt()`
and `wkhtmltoimage_extended_qt()` always return `0`; `use_graphics` is retained
by the initialization functions and ignored. Supply exactly one object to
`wkhtmltopdf_add_object`; zero or multiple objects cause conversion to return
failure and emit one failed completion callback.

Rebuild C++ consumers: settings layouts and interfaces changed. Unused C++
parameters were removed from the loader/factory (`dpi`, `mainLoader`), page-printer factory
(`painter`) and argument parsing/handlers (per-object address context).
`HeaderFooter`, `TableOfContent`, `Outline` and their helper functions are gone.

## Verification

After building the library and both executables, run:

```sh
python3 tests/option_removal_smoke.py --bin-dir build/bin
```

The smoke checks cover rejected options, retained error policies, stdin/stdout
conversion, PDF/image output, C API settings and single completion callbacks. They also check
rejection of multi-object syntax and confirm multi-page output from one HTML
without interactive form fields, bookmarks or clickable PDF link annotations.

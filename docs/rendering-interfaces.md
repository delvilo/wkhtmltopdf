# Qt5/WebKit rendering interfaces

The PDF and image converters use internal interfaces for resource loading, DOM
access, page printing and image painting. Standard Qt5/WebKit is the only
backend; Qt4 and patched Qt are removed. Public C function signatures are
retained, but removed setting keys return failure and C++ consumers must rebuild.

| Contract | Responsibility | Implementation |
| --- | --- | --- |
| `ResourceLoader` | Load URL/file/stdin/memory inputs, report progress/errors/completion, release resources | `MultiPageLoader`, including the network manager, cookies, proxy, SSL, authentication, POST and JavaScript readiness |
| `DomDocument` / `DomElement` | Query title/URLs and elements, read attributes/text, update styles | `WebKitPage` and shared opaque `QWebElement` handles |
| `PagePrinter` | Print one HTML document, including automatic PDF pagination | Standard `QWebFrame::print()` |
| `ImageRenderer` | Set viewport/scrollbars, measure content, set page background and paint | Standard `QWebPage` / `QWebFrame` |

`RenderPage` groups a loaded page's DOM, image and printing services. The factory
in `src/lib/renderbackend.cc` selects the loader implementation. Native WebKit
operations are confined to `multipageloader_p.hh`, `multipageloader.cc` and the
`webkitpage` adapter.

The PDF converter stores exactly one HTML input. Zero or multiple inputs fail
before loading or output; skipping the only failed input also fails. Whole-document
printing uses Qt5 PrintSupport, followed by file, stdout or memory delivery.
There are no TOC/header/footer loaders, page maps, outline/link rendering,
`QWebPrinter`, XML Patterns or patch capability guards.

Standard Qt5/WebKit can emit URL annotations even without the old wkhtmltopdf
patches. `WebKitPagePrinter` therefore uses a public `QPaintEngine` adapter that
forwards visual operations to the PDF painter while reporting a custom engine
type to WebKit. `QPrinter::setEngines` shares the real printer's paper settings
and page transitions. This suppresses PDF links without modifying HTML/CSS,
rasterizing the document, using private Qt headers or postprocessing PDF bytes.

Local comparison against the previous Qt5 baseline covered default, landscape,
custom paper/DPI and grayscale output: all 12 fixture pages had identical pixels
and extracted text. The fixture included print CSS, styled links, an iframe,
clipping, gradients, transforms and SVG. Its generated PDF had no link annotations.

Image validation, smart width, cropping, encoding and transactional file output
remain in the image converter. Smart width and transparency are standard Qt5
features and appear in help without unsupported-option warnings. `--zoom` uses
`QWebFrame::setZoomFactor`; PDF DPI is independent of layout zoom. The loader
and factory no longer accept DPI or auxiliary-loader arguments.

## Ownership and execution

- Converters own loaders through `QScopedPointer<ResourceLoader>`.
- A loader owns its `LoaderObject` and `RenderPage`. DOM/image services and
  element handles are borrowed; release them before clearing the loader.
- Copying a `DomElement` refers to the same element. Unmatched/default handles
  are null and return empty strings when read.
- The caller owns `createPrinter(QPrinter*)`. Destroy the returned printer before
  its loaded page and `QPrinter`. The painter argument and element-location/page
  spooling APIs are removed; Qt5 handles pagination internally.
- Loading remains event driven through QObject signals. DOM reads, image painting
  and PDF printing are synchronous on the application thread. A future backend
  must address its own threading and asynchronous behavior.

## Regression checks

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The HTTP fixture checks redirects, headers and propagation, cookies and
cookie-jar persistence, basic authentication, POST, HTTP errors, user stylesheets,
JavaScript readiness and disabled scripts on loopback.

C++ tests exercise real DOM handles, selectors, content sizing, transparency and
three-page printing. CLI/C API checks cover one HTML producing multiple pages,
rejection of multiple inputs and removed settings, absence of PDF link/outline
annotations, completion callbacks, smart/fixed image width, transparency, zoom,
cropping and output failures. No test requires a patched Qt build.

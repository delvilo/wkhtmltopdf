# WebKit rendering interfaces

The PDF and image converters use internal interfaces for resource loading, DOM
access, page printing and image painting. The selected implementation remains
Qt WebKit. The Qt dependencies, CLI options and public C API signatures are
unchanged; no Qt6 or WebEngine implementation is introduced.

| Contract | Responsibility | Current implementation |
| --- | --- | --- |
| `ResourceLoader` | Add URL/file/stdin/memory inputs, start loading, report progress/errors/completion, release resources | `MultiPageLoader`, including the existing network manager, cookies, proxy, SSL, authentication, POST and JavaScript readiness handling |
| `DomDocument` / `DomElement` | Query title/URLs, find elements, read attributes/text and update styles; retain element identity for links and headings | `WebKitPage` and opaque, shared `QWebElement` handles |
| `PagePrinter` | Print a whole document, or paginate, locate elements and spool pages when patched Qt is available | `QWebFrame::print()` and the existing patched `QWebPrinter` |
| `ImageRenderer` | Set viewport/scrollbar policy, measure content, change the page background and paint | `QWebPage` / `QWebFrame` |

`RenderPage` groups a loaded page's DOM, image and printing services. The factory
in `src/lib/renderbackend.cc` is the single place where converters select the
loader implementation. WebKit operations are confined to `multipageloader_p.hh`,
`multipageloader.cc` and the `webkitpage` adapter; the PDF, image and outline
algorithms use the contracts in `rendering.hh` and `resourceloader.hh`.

Image validation, smart width, cropping, encoding and transactional file output
remain in the image conversion layer. PDF settings, header/footer placement,
link resolution, outline construction and TOC generation remain in the PDF
layer. In particular, TOC generation still needs `QXmlQuery` and the
`xmlpatterns` module.

## Ownership and execution

- Converters own their loaders through `QScopedPointer<ResourceLoader>`.
- A loader owns each `LoaderObject` and its `RenderPage`. The references returned
  by `dom()` and `image()` are borrowed, as are element handles. Release all
  handles and page printers before clearing their loader.
- Copying a `DomElement` keeps the same DOM element; it does not copy the HTML.
  A default or unmatched element is null, and its reads return empty strings.
- The caller owns the result of `createPrinter()`. Destroy it before its page,
  `QPainter` or `QPrinter`. A pagination painter must be active when supplied.
- Resource loading remains event driven, with the same QObject signals.
  DOM reads, image painting and printing remain synchronous on the application
  thread. These contracts do not emulate WebEngine's asynchronous operations.
- `printDocument()` works on the unpatched Qt5 baseline. `supportsPagination()`
  is true only for a patched build with a pagination painter. Page numbers used
  by `spoolPage()` and `elementLocation()` are one based; an unavailable location
  has page number `-1`. Whole-document printing must not run during an active
  pagination session.

The existing patched/unpatched feature guards are retained. `webkitfeatures.hh`
explicitly imports the legacy capability macro from `QWebFrame`, where patched
Qt defines it. This compile-time dependency remains until backend capabilities
and Qt's output extensions can be decoupled. The PDF layer still uses Qt
PrintSupport and patched Qt's link/outline drawing extensions. A later
engine migration would need to address these output APIs and asynchronous
execution separately; adding these interfaces does not promise feature parity
with another rendering engine.

## Regression checks

Build the shared library and both executables, then run:

```sh
export LD_LIBRARY_PATH="$PWD/bin${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_QPA_PLATFORM=offscreen
python3 tests/image_entry_smoke.py --bin-dir bin
python3 tests/option_removal_smoke.py --bin-dir bin
python3 tests/rendering_backend_smoke.py --bin-dir bin
mkdir -p build/rendering-tests
cd build/rendering-tests
qmake ../../tests/rendering_interfaces.pro
make
./rendering_interfaces
```

The HTTP fixture uses only loopback and checks redirects, extra headers,
subresource header propagation, cookies and cookie-jar persistence, basic
authentication, POST bodies, HTTP errors, user stylesheets, JavaScript readiness
and disabled scripts for the converter paths.

The C++ tests exercise real WebKit-backed interfaces, including copied DOM
handles, heading/link selectors, null elements, content sizing, transparent
painting and a three-page PDF. Patched builds additionally check page counts,
element locations and page spooling. The existing patched-only smoke test covers
multi-document output with headers and a TOC; it is skipped on unpatched Qt.

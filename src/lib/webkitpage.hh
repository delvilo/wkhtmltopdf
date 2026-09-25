// Adapter for a page owned by the existing WebKit resource loader.
// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.

#ifndef WKHTMLTOPDF_WEBKITPAGE_HH
#define WKHTMLTOPDF_WEBKITPAGE_HH

#include "rendering.hh"
class QWebPage;

#include "dllbegin.inc"
namespace wkhtmltopdf {

class DLL_LOCAL WebKitPage: public RenderPage {
public:
	explicit WebKitPage(QWebPage & page);
	~WebKitPage();
	DomDocument & dom();
	ImageRenderer & image();
	void applySettings(const settings::Web & settings);
	PagePrinter * createPrinter(QPrinter * printer);
private:
	Q_DISABLE_COPY(WebKitPage)
	class Private;
	Private * d;
};

}
#include "dllend.inc"
#endif

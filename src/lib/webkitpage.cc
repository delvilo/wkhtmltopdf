// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.
#include "webkitpage.hh"
#include "resourceloader.hh"
#include <QPainter>
#include <QPrinter>
#include <QScopedPointer>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#include <QWebSettings>

namespace wkhtmltopdf {

class WebKitElement: public DomElement::Data {
public:
	explicit WebKitElement(const QWebElement & value): element(value) {}
	bool isNull() const { return element.isNull(); }
	QString attribute(const QString & name) const { return element.attribute(name); }
	QString tagName() const { return element.tagName(); }
	QString toPlainText() const { return element.toPlainText(); }
	void setStyleProperty(const QString & name, const QString & value) {
		element.setStyleProperty(name, value);
	}
	QWebElement element;
};

class WebKitPage::Private: public DomDocument, public ImageRenderer {
public:
	explicit Private(QWebPage & p): page(p) {}
	QWebPage & page;
	QString title() const { return page.mainFrame()->title(); }
	QUrl url() const { return page.mainFrame()->url(); }
	QUrl baseUrl() const { return page.mainFrame()->baseUrl(); }
	DomElement findFirstElement(const QString & selector) const {
		return DomElement(new WebKitElement(page.mainFrame()->findFirstElement(selector)));
	}
	QList<DomElement> findAllElements(const QString & selector) const {
		QList<DomElement> elements;
		foreach (const QWebElement & element, page.mainFrame()->findAllElements(selector))
			elements.append(DomElement(new WebKitElement(element)));
		return elements;
	}
	void setViewportSize(const QSize & size) { page.setViewportSize(size); }
	QSize viewportSize() const { return page.viewportSize(); }
	QSize contentsSize() const { return page.mainFrame()->contentsSize(); }
	void setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy) {
		page.mainFrame()->setScrollBarPolicy(orientation, policy);
	}
	int scrollBarMaximum(Qt::Orientation orientation) const {
		return page.mainFrame()->scrollBarMaximum(orientation);
	}
	void setTransparentBackground() {
		QPalette palette = page.palette();
		palette.setBrush(QPalette::Base, Qt::transparent);
		page.setPalette(palette);
	}
	void render(QPainter * painter) { page.mainFrame()->render(painter); }
};

class WebKitPagePrinter: public PagePrinter {
public:
	WebKitPagePrinter(QWebPage & page, QPrinter * printer, QPainter * painter):
		page(page), printer(printer)
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
		, pagination(painter ? new QWebPrinter(page.mainFrame(), printer, *painter) : 0)
#endif
	{
		Q_UNUSED(painter);
	}
	void printDocument() { page.mainFrame()->print(printer); }
	bool supportsPagination() const {
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
		return !pagination.isNull();
#else
		return false;
#endif
	}
	int pageCount() const {
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
		return pagination ? pagination->pageCount() : 0;
#else
		return 0;
#endif
	}
	QPair<int, QRectF> elementLocation(const DomElement & element) const {
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
		const WebKitElement * value = dynamic_cast<const WebKitElement *>(elementData(element));
		if (pagination && value) return pagination->elementLocation(value->element);
#else
		Q_UNUSED(element);
#endif
		return qMakePair(-1, QRectF());
	}
	void spoolPage(int pageNumber) {
		Q_ASSERT(supportsPagination());
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
		if (pagination) pagination->spoolPage(pageNumber);
#else
		Q_UNUSED(pageNumber);
#endif
	}
private:
	QWebPage & page;
	QPrinter * printer;
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
	QScopedPointer<QWebPrinter> pagination;
#endif
};

WebKitPage::WebKitPage(QWebPage & page): d(new Private(page)) {}
WebKitPage::~WebKitPage() { delete d; }
DomDocument & WebKitPage::dom() { return *d; }
ImageRenderer & WebKitPage::image() { return *d; }
PagePrinter * WebKitPage::createPrinter(QPrinter * printer, QPainter * painter) {
	return new WebKitPagePrinter(d->page, printer, painter);
}

void WebKitPage::applySettings(const settings::Web & s) {
	QWebSettings * ws = d->page.settings();
	if (!s.defaultEncoding.isEmpty())
		ws->setDefaultTextEncoding(s.defaultEncoding);
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
	if (!s.enableIntelligentShrinking) {
		ws->setPrintingMaximumShrinkFactor(1.0);
		ws->setPrintingMinimumShrinkFactor(1.0);
	}
#endif
	ws->setAttribute(QWebSettings::JavaEnabled, false);
	ws->setAttribute(QWebSettings::JavascriptEnabled, s.enableJavascript);
	ws->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
	ws->setAttribute(QWebSettings::JavascriptCanAccessClipboard, false);
	ws->setFontSize(QWebSettings::MinimumFontSize, s.minimumFontSize);
	ws->setAttribute(QWebSettings::PrintElementBackgrounds, s.background);
	ws->setAttribute(QWebSettings::AutoLoadImages, s.loadImages);
	ws->setAttribute(QWebSettings::PluginsEnabled, false);
	if (!s.userStyleSheet.isEmpty())
		ws->setUserStyleSheetUrl(ResourceLoader::guessUrlFromString(s.userStyleSheet));
}

}

// Rendering contracts for the existing WebKit backend.
// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.

#ifndef WKHTMLTOPDF_RENDERING_HH
#define WKHTMLTOPDF_RENDERING_HH

#include <QList>
#include <QPair>
#include <QRectF>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QUrl>
#include "websettings.hh"

class QPainter;
class QPrinter;

#include "dllbegin.inc"
namespace wkhtmltopdf {

// Copyable, opaque element handle. Handles must not outlive their loaded page.
class DLL_LOCAL DomElement {
public:
	class Data {
	public:
		virtual ~Data() {}
		virtual bool isNull() const = 0;
		virtual QString attribute(const QString & name) const = 0;
		virtual QString tagName() const = 0;
		virtual QString toPlainText() const = 0;
		virtual void setStyleProperty(const QString & name, const QString & value) = 0;
	};

	DomElement() {}
	explicit DomElement(Data * data): d(data) {}
	bool isNull() const { return !d || d->isNull(); }
	QString attribute(const QString & name) const { return d ? d->attribute(name) : QString(); }
	QString tagName() const { return d ? d->tagName() : QString(); }
	QString toPlainText() const { return d ? d->toPlainText() : QString(); }
	void setStyleProperty(const QString & name, const QString & value) {
		if (d) d->setStyleProperty(name, value);
	}

private:
	QSharedPointer<Data> d;
	friend class PagePrinter;
};

class DLL_LOCAL DomDocument {
public:
	virtual ~DomDocument() {}
	virtual QString title() const = 0;
	virtual QUrl url() const = 0;
	virtual QUrl baseUrl() const = 0;
	virtual DomElement findFirstElement(const QString & selector) const = 0;
	virtual QList<DomElement> findAllElements(const QString & selector) const = 0;
};

// Layout and paint operations are synchronous in this WebKit baseline.
class DLL_LOCAL ImageRenderer {
public:
	virtual ~ImageRenderer() {}
	virtual void setViewportSize(const QSize & size) = 0;
	virtual QSize viewportSize() const = 0;
	virtual QSize contentsSize() const = 0;
	virtual void setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy) = 0;
	virtual int scrollBarMaximum(Qt::Orientation orientation) const = 0;
	virtual void setTransparentBackground() = 0;
	virtual void render(QPainter * painter) = 0;
};

class DLL_LOCAL PagePrinter {
public:
	virtual ~PagePrinter() {}
	virtual void printDocument() = 0;
	// Only patched WebKit supports element-aware pagination. Unpatched builds
	// support printDocument(); they report false here and no page locations.
	virtual bool supportsPagination() const = 0;
	virtual int pageCount() const = 0;
	virtual QPair<int, QRectF> elementLocation(const DomElement & element) const = 0;
	virtual void spoolPage(int page) = 0;
protected:
	// Only printer implementations need access to an element's backend data.
	const DomElement::Data * elementData(const DomElement & element) const { return element.d.data(); }
};

// Page lifetime belongs to ResourceLoader. DOM/image services are borrowed;
// createPrinter transfers ownership to the caller. Destroy printers before
// their page, QPrinter or QPainter. A painter is required for pagination.
class DLL_LOCAL RenderPage {
public:
	virtual ~RenderPage() {}
	virtual DomDocument & dom() = 0;
	virtual ImageRenderer & image() = 0;
	virtual void applySettings(const settings::Web & settings) = 0;
	virtual PagePrinter * createPrinter(QPrinter * printer, QPainter * painter = 0) = 0;
};

}
#include "dllend.inc"
#endif

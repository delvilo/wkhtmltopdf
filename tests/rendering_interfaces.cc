// Integration tests for the synchronous WebKit rendering contracts.
#include "webkitpage.hh"
#include <QApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPrinter>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QtTest>
#include <QWebFrame>
#include <QWebPage>

using namespace wkhtmltopdf;

class RenderingInterfaces: public QObject {
	Q_OBJECT
private slots:
	void domHandlesAndQueries() {
		QWebPage native;
		QSignalSpy loaded(&native, SIGNAL(loadFinished(bool)));
		native.mainFrame()->setHtml("<html><head><title>DOM contract</title></head><body>"
			"<h1 id='first'>First heading</h1><h2>Second heading</h2>"
			"<a name='target' href='next.html#part'>Next</a></body></html>",
			QUrl("http://example.test/base/"));
		QTRY_VERIFY(!loaded.isEmpty());
		WebKitPage page(native);
		DomDocument & dom = page.dom();
		QCOMPARE(dom.title(), QString("DOM contract"));
		QCOMPARE(dom.baseUrl(), QUrl("http://example.test/base/"));
		QList<DomElement> headings = dom.findAllElements("h1,h2,h3");
		QCOMPARE(headings.size(), 2);
		QCOMPARE(headings[0].tagName(), QString("H1"));
		QCOMPARE(headings[1].toPlainText(), QString("Second heading"));
		DomElement original = dom.findFirstElement("#first");
		DomElement copy = original;
		copy.setStyleProperty("color", "red");
		QVERIFY(original.attribute("style").contains("red"));
		QCOMPARE(dom.findFirstElement("a").attribute("href"), QString("next.html#part"));
		QVERIFY(dom.findFirstElement("#missing").isNull());
		DomElement missing;
		QVERIFY(missing.isNull());
		QVERIFY(missing.toPlainText().isEmpty());
		missing.setStyleProperty("color", "red");
	}

	void imageLayoutAndTransparentPainting() {
		QWebPage native;
		QSignalSpy loaded(&native, SIGNAL(loadFinished(bool)));
		native.mainFrame()->setHtml("<html><body style='margin:0;background:#123456'>"
			"<div style='width:20px;height:20px;background:#ff0000'></div>"
			"<div style='height:220px'></div></body></html>");
		QTRY_VERIFY(!loaded.isEmpty());
		WebKitPage page(native);
		ImageRenderer & renderer = page.image();
		renderer.setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
		renderer.setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
		renderer.setViewportSize(QSize(100, 80));
		QCOMPARE(renderer.viewportSize(), QSize(100, 80));
		QVERIFY(renderer.contentsSize().height() >= 240);
		QCOMPARE(renderer.scrollBarMaximum(Qt::Horizontal), 0);
		QImage image(100, 80, QImage::Format_ARGB32_Premultiplied);
		image.fill(0);
		QPainter painter(&image);
		renderer.render(&painter);
		QVERIFY(painter.end());
		QCOMPARE(image.pixel(50, 50), qRgb(0x12, 0x34, 0x56));
		DomElement body = page.dom().findFirstElement("body");
		body.setStyleProperty("background-color", "transparent");
		body.setStyleProperty("background-image", "none");
		renderer.setTransparentBackground();
		image.fill(0);
		QVERIFY(painter.begin(&image));
		renderer.render(&painter);
		QVERIFY(painter.end());
		QCOMPARE(qAlpha(image.pixel(50, 50)), 0);
		QCOMPARE(image.pixel(10, 10), qRgb(255, 0, 0));
	}

	void documentPrintingAndPaginationCapability() {
		QWebPage native;
		QSignalSpy loaded(&native, SIGNAL(loadFinished(bool)));
		native.mainFrame()->setHtml("<html><body><h1>First</h1>"
			"<h1 style='page-break-before:always'>Second</h1>"
			"<h1 style='page-break-before:always'>Third</h1></body></html>");
		QTRY_VERIFY(!loaded.isEmpty());
		WebKitPage page(native);
		QTemporaryFile target;
		QVERIFY(target.open());
		const QString path = target.fileName();
		target.close();
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(path);
		QScopedPointer<PagePrinter> document(page.createPrinter(&printer));
		QVERIFY(!document->supportsPagination()); // No pagination painter supplied.
		document->printDocument();
		document.reset();
		QFile output(path);
		QVERIFY(output.open(QIODevice::ReadOnly));
		const QByteArray pdf = output.readAll();
		QVERIFY(pdf.startsWith("%PDF-"));
		QVERIFY(pdf.contains("%%EOF"));
		QRegExp pages("/Type\\s*/Page[\\s/>]");
		int count = 0, position = 0;
		const QString contents = QString::fromLatin1(pdf.constData(), pdf.size());
		while ((position = pages.indexIn(contents, position)) != -1) {
			++count;
			position += pages.matchedLength();
		}
		QCOMPARE(count, 3);
		output.close();
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
		QPainter painter(&printer);
		document.reset(page.createPrinter(&printer, &painter));
		QVERIFY(document->supportsPagination());
		QCOMPARE(document->pageCount(), 3);
		const QList<DomElement> headings = page.dom().findAllElements("h1");
		QCOMPARE(document->elementLocation(headings[1]).first, 2);
		document->spoolPage(1);
		document.reset();
		QVERIFY(painter.end());
#endif
	}
};

QTEST_MAIN(RenderingInterfaces)
#include "rendering_interfaces.moc"

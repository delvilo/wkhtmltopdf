// Loading contract shared by PDF and image conversion.
// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.

#ifndef WKHTMLTOPDF_RESOURCELOADER_HH
#define WKHTMLTOPDF_RESOURCELOADER_HH

#include <QFile>
#include <QObject>
#include "loadsettings.hh"
#include "rendering.hh"

#include "dllbegin.inc"
namespace wkhtmltopdf {

class DLL_LOCAL LoaderObject {
public:
	RenderPage & page;
	bool skip;
	explicit LoaderObject(RenderPage & p): page(p), skip(false) {}
};

class DLL_LOCAL ResourceLoader: public QObject {
	Q_OBJECT
public:
	virtual ~ResourceLoader() {}
	// Returned objects remain valid until clearResources() or destruction.
	// A null return indicates an input-staging failure, reported by error().
	virtual LoaderObject * addResource(const QString & url, const settings::LoadPage & settings, const QString * data = 0) = 0;
	virtual LoaderObject * addResource(const QUrl & url, const settings::LoadPage & settings) = 0;
	virtual int httpErrorCode() = 0;
	static QUrl guessUrlFromString(const QString & string);
	static bool copyFile(QFile & src, QFile & dst);
public slots:
	virtual void load() = 0;
	virtual void clearResources() = 0;
	virtual void cancel() = 0;
signals:
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void debug(QString text);
	void info(QString text);
	void warning(QString text);
	void error(QString text);
};

}
#include "dllend.inc"
#endif

// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2010-2020 wkhtmltopdf authors
//
// This file is part of wkhtmltopdf.
//
// wkhtmltopdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// wkhtmltopdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with wkhtmltopdf.  If not, see <http://www.gnu.org/licenses/>.


#include "imageconverter_p.hh"
#include "imagesettings.hh"
#include "imageoutput.hh"
#include <climits>
#include <QBuffer>
#include <QDebug>
#include <QEventLoop>
#include <QImage>
#include <QImageWriter>
#include <QObject>
#include <QPainter>
#include <QSvgGenerator>
#include <QUrl>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#include <qapplication.h>

namespace wkhtmltopdf {

ImageConverterPrivate::ImageConverterPrivate(ImageConverter & o, wkhtmltopdf::settings::ImageGlobal & s, const QString * data):
	settings(s),
	loader(s.loadGlobal, 96, true),
	out(o) {
	if (data) inputData = *data;

	phaseDescriptions.push_back("Loading page");
	phaseDescriptions.push_back("Rendering");
	phaseDescriptions.push_back("Done");

	connect(&loader, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
	connect(&loader, SIGNAL(loadFinished(bool)), this, SLOT(pagesLoaded(bool)));
	connect(&loader, SIGNAL(error(QString)), this, SLOT(forwardError(QString)));
	connect(&loader, SIGNAL(warning(QString)), this, SLOT(forwardWarning(QString)));
	connect(&loader, SIGNAL(info(QString)), this, SLOT(forwardInfo(QString)));
	connect(&loader, SIGNAL(debug(QString)), this, SLOT(forwardDebug(QString)));
}

void ImageConverterPrivate::beginConvert() {
	error = false;
	conversionDone = false;
	errorCode = 0;
	outputData.clear();
	progressString = "0%";
	currentPhase = 0;
	emit out.phaseChanged();
	loadProgress(0);

	QString message;
	if (!settings.validate(message)) {
		emit out.error(message);
		fail();
		return;
	}
	settings.fmt = settings.outputFormat();
	loaderObject = loader.addResource(settings.in, settings.loadPage, &inputData);
	updateWebSettings(loaderObject->page.settings(), settings.web);
	loader.load();
}


void ImageConverterPrivate::clearResources() {
	loader.clearResources();
}

void ImageConverterPrivate::pagesLoaded(bool ok) {
	if (conversionDone) return;
	if (errorCode == 0) errorCode = loader.httpErrorCode();
	if (!ok) {
		fail();
		return;
	}
	currentPhase = 1;
	emit out.phaseChanged();
	loadProgress(0);

	QString message;
	if (!renderImage(message)) {
		outputData.clear();
		emit out.error(message);
		fail();
		return;
	}

	loadProgress(100);
	currentPhase = 2;
	clearResources();
	emit out.phaseChanged();
	conversionDone = true;
	emit out.finished(true);
	qApp->exit(0);
}

bool ImageConverterPrivate::renderImage(QString & message) {
	QWebFrame * frame = loaderObject->page.mainFrame();
	frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
	loadProgress(25);

	// Calculate a viewport wide enough for unbreakable content.
	int highWidth = settings.screenWidth;
	loaderObject->page.setViewportSize(QSize(highWidth, 10));
	if (settings.smartWidth && frame->scrollBarMaximum(Qt::Horizontal) > 0) {
		if (highWidth < 10) highWidth = 10;
		int lowWidth = highWidth;
		while (frame->scrollBarMaximum(Qt::Horizontal) > 0 && highWidth < 32000) {
			lowWidth = highWidth;
			highWidth *= 2;
			loaderObject->page.setViewportSize(QSize(highWidth, 10));
		}
		while (highWidth - lowWidth > 10) {
			const int width = lowWidth + (highWidth - lowWidth) / 2;
			loaderObject->page.setViewportSize(QSize(width, 10));
			if (frame->scrollBarMaximum(Qt::Horizontal) > 0)
				lowWidth = width;
			else
				highWidth = width;
		}
	}
	frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	// Establish the final width before asking WebKit for the automatic height.
	loaderObject->page.setViewportSize(QSize(highWidth, 10));
	const int height = settings.screenHeight > 0 ? settings.screenHeight : frame->contentsSize().height();
	loaderObject->page.setViewportSize(QSize(highWidth, height));

	const QSize viewport = loaderObject->page.viewportSize();
	const int left = qMax(0, settings.crop.left);
	const int top = qMax(0, settings.crop.top);
	if (left >= viewport.width() || top >= viewport.height()) {
		message = "Will not output an empty image";
		return false;
	}
	// Clip before constructing QRect, avoiding overflow with large crop values.
	const int width = settings.crop.width == -1 ? viewport.width() - left :
		qMin(settings.crop.width, viewport.width() - left);
	const int cropHeight = settings.crop.height == -1 ? viewport.height() - top :
		qMin(settings.crop.height, viewport.height() - top);
	const QRect rect(left, top, width, cropHeight);
	if (rect.isEmpty()) {
		message = "Will not output an empty image";
		return false;
	}

	ImageOutput output(settings.out, outputData);
	if (!output.open()) {
		message = QString("Could not open image output: %1").arg(output.errorString());
		return false;
	}

	QImage image;
	QByteArray svgData;
	{
		QBuffer svgBuffer(&svgData);
		QPainter painter;
		QSvgGenerator generator;
		if (settings.fmt == "svg") {
			// QSvgGenerator does not propagate short writes from its device.
			// Serialize first, then check the complete write to the destination.
			if (!svgBuffer.open(QIODevice::WriteOnly)) {
				message = "Could not open the SVG buffer";
				return false;
			}
			generator.setOutputDevice(&svgBuffer);
			generator.setSize(rect.size());
			generator.setViewBox(QRect(QPoint(0, 0), rect.size()));
#ifdef __EXTENSIVE_WKHTMLTOPDF_QT_HACK__
			generator.setViewBoxClip(true);
#endif
			if (!painter.begin(&generator)) {
				message = "Could not initialize SVG rendering";
				return false;
			}
		} else {
			// Qt 4 stores the ARGB32 image byte count in an int.
			if (qint64(rect.width()) * rect.height() > INT_MAX / 4) {
				message = "Image dimensions exceed the raster allocation limit";
				return false;
			}
			image = QImage(rect.size(), QImage::Format_ARGB32_Premultiplied);
			if (image.isNull()) {
				message = "Could not allocate the output image";
				return false;
			}
			image.fill(0);
			if (!painter.begin(&image)) {
				message = "Could not initialize image rendering";
				return false;
			}
		}

		if (settings.transparent && (settings.fmt == "png" || settings.fmt == "svg")) {
			QWebElement body = frame->findFirstElement("body");
			body.setStyleProperty("background-color", "transparent");
			body.setStyleProperty("background-image", "none");
			QPalette palette = loaderObject->page.palette();
			palette.setBrush(QPalette::Base, Qt::transparent);
			loaderObject->page.setPalette(palette);
		} else {
			painter.fillRect(QRect(QPoint(0, 0), viewport), Qt::white);
		}
		painter.translate(-rect.left(), -rect.top());
		frame->render(&painter);
		if (!painter.end()) {
			message = "Could not finish rendering the image";
			return false;
		}
	}

	if (settings.fmt == "svg") {
		if (output.device()->write(svgData) != svgData.size()) {
			message = QString("Could not save SVG image: %1").arg(output.device()->errorString());
			return false;
		}
	} else {
		QImageWriter writer(output.device(), settings.fmt.toLatin1());
		writer.setQuality(settings.quality);
		if (!writer.write(image)) {
			message = QString("Could not save image: %1").arg(writer.errorString());
			return false;
		}
	}
	if (!output.commit()) {
		message = QString("Could not commit image output: %1").arg(output.errorString());
		return false;
	}
	return true;
}

Converter & ImageConverterPrivate::outer() {
	return out;
}

ImageConverter::~ImageConverter() {
	delete d;
}

ConverterPrivate & ImageConverter::priv() {
	return *d;
}


ImageConverter::ImageConverter(wkhtmltopdf::settings::ImageGlobal & s, const QString * data) {
	d = new ImageConverterPrivate(*this, s, data);
}

const QByteArray & ImageConverter::output() {
	return d->outputData;
}

}

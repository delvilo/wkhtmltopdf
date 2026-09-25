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

#include "pdfconverter_p.hh"
#include "renderbackend.hh"
#include <QApplication>
#include <QFile>
#include <QPrinter>

#include "dllbegin.inc"
using namespace wkhtmltopdf;

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

PdfConverterPrivate::PdfConverterPrivate(settings::PdfGlobal & s, PdfConverter & o):
	settings(s), pageLoader(createResourceLoader(s.load)), out(o),
	hasInput(false), multipleInputs(false), input(0) {
	phaseDescriptions << "Loading page" << "Printing pages" << "Done";
	currentPhase = 0;
	errorCode = 0;
	error = false;
	conversionDone = false;
	connect(pageLoader.data(), SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
	connect(pageLoader.data(), SIGNAL(loadFinished(bool)), this, SLOT(pagesLoaded(bool)));
	connect(pageLoader.data(), SIGNAL(error(QString)), this, SLOT(forwardError(QString)));
	connect(pageLoader.data(), SIGNAL(warning(QString)), this, SLOT(forwardWarning(QString)));
	connect(pageLoader.data(), SIGNAL(info(QString)), this, SLOT(forwardInfo(QString)));
	connect(pageLoader.data(), SIGNAL(debug(QString)), this, SLOT(forwardDebug(QString)));
}

PdfConverterPrivate::~PdfConverterPrivate() {
	clearResources();
}

void PdfConverterPrivate::beginConvert() {
	error = false;
	conversionDone = false;
	errorCode = 0;
	currentPhase = 0;
	progressString = "0%";
	outputData.clear();
	if (!hasInput || multipleInputs) {
		emit out.error("PDF conversion requires exactly one HTML input document.");
		fail();
		return;
	}
	input = pageLoader->addResource(inputSettings.page, inputSettings.load, &inputData);
	if (!input) { fail(); return; }
	input->page.applySettings(inputSettings.web);
	emit out.phaseChanged();
	loadProgress(0);
	pageLoader->load();
}

bool PdfConverterPrivate::printDocument(const QString & path) {
	// The document printer is destroyed before its QPrinter and loaded page.
	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFileName(path);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setResolution(settings.dpi);
	if (settings.margin.left.second != settings.margin.right.second ||
		settings.margin.left.second != settings.margin.top.second ||
		settings.margin.left.second != settings.margin.bottom.second) {
		emit out.error("Currently all margin units must be the same!");
		return false;
	}
	printer.setPageMargins(settings.margin.left.first, settings.margin.top.first,
		settings.margin.right.first, settings.margin.bottom.first, settings.margin.left.second);
	if (settings.size.height.first != -1 && settings.size.width.first != -1)
		printer.setPaperSize(QSizeF(settings.size.width.first, settings.size.height.first), settings.size.height.second);
	else
		printer.setPaperSize(settings.size.pageSize);
	printer.setOrientation(settings.orientation);
	printer.setColorMode(settings.colorMode);
	printer.setCreator("wkhtmltopdf " STRINGIZE(FULL_VERSION));
	printer.setDocName(settings.documentTitle.isEmpty() ? input->page.dom().title() : settings.documentTitle);
	if (!printer.isValid()) {
		emit out.error("Unable to write to destination");
		return false;
	}
	QScopedPointer<PagePrinter> document(input->page.createPrinter(&printer));
	document->printDocument();
	if (printer.printerState() == QPrinter::Error || printer.printerState() == QPrinter::Aborted) {
		emit out.error("PDF printing failed");
		return false;
	}
	return true;
}

void PdfConverterPrivate::pagesLoaded(bool ok) {
	if (conversionDone) return;
	errorCode = pageLoader->httpErrorCode();
	if (!ok || error || !input || input->skip) {
		if (input && input->skip) emit out.error("The only input document was skipped; there is nothing to print.");
		fail();
		return;
	}
	currentPhase = 1;
	emit out.phaseChanged();
	const bool temporary = settings.out.isEmpty() || settings.out == "-";
	const QString path = temporary ? tempOut.create(".pdf") : settings.out;
	if (!printDocument(path)) { fail(); return; }
	QFile result(path);
	if (!result.open(QIODevice::ReadOnly) || result.size() == 0) {
		emit out.error("Reading PDF output failed");
		fail();
		return;
	}
	if (settings.out.isEmpty()) {
		outputData = result.readAll();
	} else if (settings.out == "-") {
		QFile standardOutput;
		if (!standardOutput.open(stdout, QIODevice::WriteOnly) ||
			!ResourceLoader::copyFile(result, standardOutput) || !standardOutput.flush()) {
			emit out.error("Could not write PDF to stdout");
			result.close();
			fail();
			return;
		}
	}
	result.close();
	progressString.clear();
	emit out.progressChanged(-1);
	clearResources();
	currentPhase = 2;
	conversionDone = true;
	emit out.phaseChanged();
	emit out.finished(true);
	qApp->exit(0);
}

void PdfConverterPrivate::clearResources() {
	input = 0;
	pageLoader->clearResources();
	inputData.clear();
	hasInput = false;
	multipleInputs = false;
	tempOut.removeAll();
}

Converter & PdfConverterPrivate::outer() { return out; }

PdfConverter::PdfConverter(settings::PdfGlobal & settings):
	d(new PdfConverterPrivate(settings, *this)) {}

PdfConverter::~PdfConverter() {
	d->deleteLater();
}

void PdfConverter::addResource(const settings::PdfObject & page, const QString * data) {
	if (d->hasInput) {
		d->multipleInputs = true;
		return;
	}
	d->inputSettings = page;
	d->inputData = data ? *data : QString();
	d->hasInput = true;
}

const QByteArray & PdfConverter::output() { return d->outputData; }
const settings::PdfGlobal & PdfConverter::globalSettings() const { return d->settings; }
ConverterPrivate & PdfConverter::priv() { return *d; }

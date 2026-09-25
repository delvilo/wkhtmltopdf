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

#ifndef __PDFCONVERTER_P_HH__
#define __PDFCONVERTER_P_HH__

#include "converter_p.hh"
#include "pdfconverter.hh"
#include "resourceloader.hh"
#include "tempfile.hh"
#include <QScopedPointer>

#include "dllbegin.inc"
namespace wkhtmltopdf {

class DLL_LOCAL PdfConverterPrivate: public ConverterPrivate {
	Q_OBJECT
public:
	PdfConverterPrivate(settings::PdfGlobal & settings, PdfConverter & converter);
	~PdfConverterPrivate();

private:
	settings::PdfGlobal & settings;
	QScopedPointer<ResourceLoader> pageLoader;
	PdfConverter & out;
	settings::PdfObject inputSettings;
	QString inputData;
	bool hasInput;
	bool multipleInputs;
	LoaderObject * input;
	TempFile tempOut;
	QByteArray outputData;

	void clearResources();
	bool printDocument(const QString & path);
	virtual Converter & outer();
	friend class PdfConverter;

public slots:
	void pagesLoaded(bool ok);
	void beginConvert();
};

}
#include "dllend.inc"
#endif //__PDFCONVERTER_P_HH__

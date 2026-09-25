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

#ifndef __PDFSETTINGS_HH__
#define __PDFSETTINGS_HH__

#include <QNetworkProxy>
#include <QPrinter>
#include <QString>
#include <logging.hh>
#include <loadsettings.hh>
#include <websettings.hh>

#include <dllbegin.inc>
namespace wkhtmltopdf {
namespace settings {

typedef QPair<qreal, QPrinter::Unit> UnitReal;

/*! \brief Settings considering margins */
struct DLL_PUBLIC Margin {
	Margin();
	//!Margin applied to the top of the page
	UnitReal top;
	//!Margin applied to the right of the page
	UnitReal right;
	//!Margin applied to the bottom of the page
	UnitReal bottom;
	//!Margin applied to the leftp of the page
	UnitReal left;
};

/*! \brief Settings considering page size */
struct DLL_PUBLIC Size {
	Size();
	//! What size paper should we use
	QPrinter::PageSize pageSize;
	//!Height of the page
	UnitReal height;
	//!Width of the page
	UnitReal width;
};

/*! \brief Class holding all user setting.
    This class holds all the user settings, settings can be filled in by hand,
    or with other methods.
    \sa CommandLineParser::parse()
*/
struct DLL_PUBLIC PdfGlobal {
	PdfGlobal();

	//! Size related settings
	Size size;

	//! Log level
	LogLevel logLevel;

	//! Should we orientate in landscape or portrate
	QPrinter::Orientation orientation;

	//! Color or grayscale
	QPrinter::ColorMode colorMode;

	//! What dpi should be used when printing
	int dpi;

	//! The file where in to store the output
	QString out;

	QString documentTitle;

	//! Margin related settings
	Margin margin;

	LoadGlobal load;

	QString get(const char * name);
	bool set(const char * name, const QString & value);
};

struct DLL_PUBLIC PdfObject {
	QString page;
	LoadPage load;
	Web web;
	QString get(const char * name);
	bool set(const char * name, const QString & value);
};

DLL_PUBLIC QPrinter::PageSize strToPageSize(const char * s, bool * ok=0);
DLL_PUBLIC QString pageSizeToStr(QPrinter::PageSize ps);

DLL_PUBLIC UnitReal strToUnitReal(const char * s, bool * ok=0);
DLL_PUBLIC QString unitRealToStr(const UnitReal & ur, bool * ok);

DLL_PUBLIC QPrinter::Orientation strToOrientation(const char * s, bool * ok=0);
DLL_PUBLIC QString orientationToStr(QPrinter::Orientation o);

DLL_PUBLIC QPrinter::ColorMode strToColorMode(const char * s, bool * ok=0);
DLL_PUBLIC QString colorModeToStr(QPrinter::ColorMode o);

}

}
#include <dllend.inc>
#endif //__PDFSETTINGS_HH__

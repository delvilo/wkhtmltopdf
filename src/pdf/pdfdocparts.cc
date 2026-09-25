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

#include "outputter.hh"
#include "pdfcommandlineparser.hh"

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

/*!
  Output name and a short description
  \param o The outputter to output to
*/
void PdfCommandLineParser::outputManName(Outputter * o) const {
	o->beginSection("Name");
	o->paragraph("wkhtmltopdf - html to pdf converter");
	o->endSection();
}

/*!
  Output a short synopsis on how to call the command line program
  \param o The outputter to output to
*/
void PdfCommandLineParser::outputSynopsis(Outputter * o) const {
	o->beginSection("Synopsis");
	o->verbatim("wkhtmltopdf [OPTION]... <input HTML URL/file> <output PDF>\n");
	o->paragraph("Exactly one HTML input is accepted. Use - for stdin or stdout. "
		"Page options may also follow the input; global options must precede it.");
	o->endSection();
}

void PdfCommandLineParser::outputDescripton(Outputter * o) const {
	o->beginSection("Description");
	o->paragraph("Converts one HTML document to PDF using Qt5/WebKit on Linux. "
		"Long HTML documents are automatically split into PDF pages. "
		"The offscreen platform is selected by default for headless operation.");
	o->endSection();
}

void PdfCommandLineParser::outputPageBreakDoc(Outputter * o) const {
	o->beginSection("Page Breaking");
	o->paragraph("Qt5/WebKit handles pagination and print CSS. Arrange long documents "
		"with suitable page-break-before, page-break-after and page-break-inside rules, "
		"and check the output for content taller than a page. Layout zoom and paper "
		"margins are available; WebKit's automatic print shrinking is not configurable.");
	o->endSection();
}

void PdfCommandLineParser::outputPageSizes(Outputter * o) const {
	o->beginSection("Page sizes");
	o->beginParagraph();
	o->text("The default page size of the rendered document is A4, but by using the --page-size "
			"option this can be changed to almost anything else, such as: A3, Letter and Legal.  "
			"For a full list of supported pages sizes please see ");
	o->link("https://doc.qt.io/qt-5/qpagesize.html#PageSizeId-enum");
	o->text(".");
	o->endParagraph();
	o->paragraph("For a more fine grained control over the page size the "
				 "--page-height and --page-width options may be used");
	o->endSection();
}


void PdfCommandLineParser::outputContact(Outputter * o) const {
	o->beginSection("Contact");
	o->beginParagraph();
	o->text("If you experience bugs or want to request new features please visit ");
	o->link("https://wkhtmltopdf.org/support.html");
	o->endParagraph();
	o->endSection();
}

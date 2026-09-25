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

#include "imagecommandlineparser.hh"
#include "outputter.hh"
#include <webkitfeatures.hh>

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

/*!
  Output name and a short description
  \param o The outputter to output to
*/
void ImageCommandLineParser::outputManName(Outputter * o) const {
	o->beginSection("Name");
	o->paragraph("wkhtmltoimage - html to image converter");
	o->endSection();
}

/*!
  Output a short synopsis on how to call the command line program
  \param o The outputter to output to
*/
void ImageCommandLineParser::outputSynopsis(Outputter * o) const {
	o->beginSection("Synopsis");
	o->verbatim("wkhtmltoimage [OPTIONS]... <input file> <output file>\n");
	o->endSection();
}

/*!
  Explain what the program does
  \param o The outputter to output to
*/
void ImageCommandLineParser::outputDescripton(Outputter * o) const {
	o->beginSection("Description");
	o->beginParagraph();
	o->text("Converts an HTML page into an image, ");
	o->endParagraph();
	o->endSection();
}


/*!
  Output contact information
  \param o The outputter to output to
*/
void ImageCommandLineParser::outputContact(Outputter * o) const {
	o->beginSection("Contact");
	o->beginParagraph();
	o->text("If you experience bugs or want to request new features please visit ");
	o->link("https://wkhtmltopdf.org/support.html");
	o->endParagraph();
	o->endSection();
}

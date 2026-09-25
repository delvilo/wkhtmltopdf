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

using namespace wkhtmltopdf::settings;
/*!
  \file commandlineparser.hh
  \brief Defines the PdfCommandLineParser class
*/

/*!
  \file commandlineparser_p.hh
  \brief Defines the PdfCommandLineParserPrivate, ArgHandler and Outputter class
*/

/*!
  Output the man page to a given file
  \param fd The file to store the man page
*/
void PdfCommandLineParser::manpage(FILE * fd) const {
	Outputter * o = Outputter::man(fd);
 	outputManName(o);
 	outputSynopsis(o);
 	outputDescripton(o);
	outputSwitches(o, true);
	outputProxyDoc(o);
	outputPageSizes(o);
 	outputPageBreakDoc(o);
 	outputContact(o);
	delete o;
}

/*!
  Output usage information aka. --help
  \param fd The file to output the information to
  \param extended Should we show extended arguments
*/
void PdfCommandLineParser::usage(FILE * fd, bool extended) const {
	Outputter * o = Outputter::text(fd);
	outputName(o);
	outputSynopsis(o);
 	outputDescripton(o);
	outputSwitches(o, extended);
	if (extended) {
		outputPageSizes(o);
		outputProxyDoc(o);
	}
 	outputContact(o);
	delete o;
}

// Accept page options before or after the input, and global options before it.
void PdfCommandLineParser::parseArguments(int argc, const char * const * argv) {
	bool defaultMode = false;
	int arg = 1;
	for (; arg < argc; ++arg) {
		if (argv[arg][0] != '-' || argv[arg][1] == '\0' || defaultMode) break;
		parseArg(global | page, argc, argv, defaultMode, arg);
	}
	if (arg < argc && (!strcmp(argv[arg], "cover") || !strcmp(argv[arg], "toc") || !strcmp(argv[arg], "page"))) {
		fprintf(stderr, "Document object commands have been removed; specify one HTML input and one PDF output.\n");
		exit(1);
	}
	if (arg < argc) pageSettings.page = QString::fromLocal8Bit(argv[arg++]);
	for (; arg < argc - 1; ++arg) {
		if (argv[arg][0] != '-' || argv[arg][1] == '\0' || defaultMode) break;
		parseArg(page, argc, argv, defaultMode, arg);
	}
	if (pageSettings.page.isEmpty() || arg != argc - 1) {
		fprintf(stderr, "You need to specify exactly one HTML input file and one PDF output file.\nUse - for stdin or stdout.\n\n");
		usage(stderr, false);
		exit(1);
	}
	globalSettings.out = QString::fromLocal8Bit(argv[arg]);
}

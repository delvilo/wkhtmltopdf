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
#include "progressfeedback.hh"
#include "cliapplication.hh"
#include <imageconverter.hh>
#include <imagesettings.hh>
#include <utilities.hh>

int main(int argc, char** argv) {
	wkhtmltopdf::CliApplication::prepareEnvironment();
	//This will store all our settings
	wkhtmltopdf::settings::ImageGlobal settings;
	//Create a command line parser to parse commandline arguments
	ImageCommandLineParser parser(settings);
	//Parse the arguments
	parser.parseArguments(argc, argv);

	wkhtmltopdf::CliApplication app(argc, argv);

	//Create the actual page converter to convert the pages
	wkhtmltopdf::ImageConverter converter(settings);

	wkhtmltopdf::ProgressFeedback feedback(settings.logLevel, converter);
	bool success = converter.convert();
	return handleError(success, converter.httpErrorCode());
}

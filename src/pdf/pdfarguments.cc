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

#include "arghandler.inl"
#include "pdfcommandlineparser.hh"
#include <qglobal.h>

/*!
  \class ArgHandler
  \brief Class responsible for handling an argument
*/

/*!
  \var ArgHandler::longName
  \brief The long name of the argument, e.g. "help" for "--help"
*/

/*!
  \var ArgHandler::desc
  \brief A descriptive text of the argument
*/

/*!
  \var ArgHandler::shortSwitch
  \brief Sort name, e.g. 'h' for '-h', if 0 there is no short name
*/

/*!
  \var ArgHandler::argn
  \brief The names of the arguments to the switch
*/

/*!
  \var ArgHandler::display
  \brief Indicate that the argument is not hidden
*/

/*!
  \var ArgHandler::extended
  \brief Indicate if the argument is an extended argument
*/

/*!
  \fn ArgHandler::operator()(const char * const * args, CommandLineParserPrivate & parser)
  Callend when the switch was specified
  \param args The arguments to the switch, guarantied to have size of argn
  \param settings The settings to store the information in
*/


/*!
  \class CommandLineParserPrivate
  Implementation details for CommandLineParser
*/

using namespace wkhtmltopdf::settings;

struct UnitRealTM: public SomeSetterTM<UnitReal> {
	static UnitReal strToT(const char * val, bool &ok) {
		return strToUnitReal(val, &ok);
	}
	static QString TToStr(const UnitReal & u, bool & ok) {
		return unitRealToStr(u, &ok);
	}
};
/*!
  Argument handler setting a real-number/unit combo variable
 */
typedef SomeSetter<UnitRealTM> UnitRealSetter;

struct PageSizeTM: public SomeSetterTM<QPrinter::PageSize> {
	static QPrinter::PageSize strToT(const char * val, bool &ok) {
		return strToPageSize(val, &ok);
	}
	static QString TToStr(const QPrinter::PageSize & s, bool & ok) {
		ok=true;
		return pageSizeToStr(s);
	}
};
/*!
  Argument handler setting a page size variable
 */
typedef SomeSetter<PageSizeTM> PageSizeSetter;

struct OrientationTM: public SomeSetterTM<QPrinter::Orientation> {
	static QPrinter::Orientation strToT(const char * val, bool &ok) {
		return strToOrientation(val, &ok);
	}
	static QString TToStr(const QPrinter::Orientation & o, bool & ok) {
		ok=true;
		return orientationToStr(o);
	}
};
/*!
  Argument handler setting a orientation variable
 */
typedef SomeSetter<OrientationTM> OrientationSetter;

/*!
  Construct the commandline parser adding all the arguments
  \param s The settings to store values in
*/
PdfCommandLineParser::PdfCommandLineParser(PdfGlobal & s, PdfObject & ps):
	globalSettings(s),
	pageSettings(ps) {
	section("Global Options");
	mode(global);

	addDocArgs();

	extended(false);

	addarg("log-level", 0, "Set log level to: none, error, warn, info or debug", new LogLevelSetter(s.logLevel, "level"));

	addarg("orientation",'O',"Set orientation to Landscape or Portrait", new OrientationSetter(s.orientation, "orientation"));
	addarg("page-size",'s',"Set paper size to: A4, Letter, etc.", new PageSizeSetter(s.size.pageSize, "Size"));

	addarg("grayscale",'g',"PDF will be generated in grayscale", new ConstSetter<QPrinter::ColorMode>(s.colorMode,QPrinter::GrayScale));

	addarg("title", 0, "The title of the generated pdf file (The HTML document title is used if not specified)", new QStrSetter(s.documentTitle,"text"));

	extended(true);
	addarg("margin-bottom",'B',"Set the page bottom margin", new UnitRealSetter(s.margin.bottom,"unitreal"));
	addarg("margin-left",'L',"Set the page left margin", new UnitRealSetter(s.margin.left,"unitreal"));
	addarg("margin-right",'R',"Set the page right margin", new UnitRealSetter(s.margin.right,"unitreal"));
	addarg("margin-top",'T',"Set the page top margin", new UnitRealSetter(s.margin.top,"unitreal"));

	addarg("dpi",'d',"Set PDF print resolution", new IntSetter(s.dpi,"dpi"));
	addarg("page-height", 0, "Page height", new UnitRealSetter(s.size.height,"unitreal"));
	addarg("page-width", 0, "Page width", new UnitRealSetter(s.size.width,"unitreal"));

	addGlobalLoadArgs(s.load);


	section("Page Options");
	mode(page);
	addWebArgs(ps.web);
	extended(true);
	addarg("no-background",0,"Do not print background", new ConstSetter<bool>(ps.web.background, false));
	addarg("background",0,"Do print background", new ConstSetter<bool>(ps.web.background, true));
	addPageLoadArgs(ps.load);
}

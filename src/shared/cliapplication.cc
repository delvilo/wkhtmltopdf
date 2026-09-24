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

#include "cliapplication.hh"
#include <utilities.hh>

#if defined(Q_OS_UNIX)
#include <locale.h>
#include <stdlib.h>
#endif

namespace wkhtmltopdf {

void CliApplication::prepareEnvironment() {
#if defined(Q_OS_UNIX)
	setlocale(LC_ALL, "");
#if QT_VERSION >= 0x050000 && !defined(__EXTENSIVE_WKHTMLTOPDF_QT_HACK__)
	// Respect an explicit platform selected by the caller.
	setenv("QT_QPA_PLATFORM", "offscreen", 0);
#endif
#endif
}

#if QT_VERSION < 0x050000
bool CliApplication::useGraphics() {
#if (defined(Q_OS_UNIX) || defined(Q_OS_MAC)) && defined(__EXTENSIVE_WKHTMLTOPDF_QT_HACK__)
	QApplication::setGraphicsSystem("raster");
	return false;
#else
	return true;
#endif
}
#endif

CliApplication::CliApplication(int & argc, char ** argv):
#if QT_VERSION < 0x050000
	QApplication(argc, argv, useGraphics())
#else
	QApplication(argc, argv)
#endif
{
	// QApplication takes ownership of the style.
	setStyle(new MyLooksStyle());
}

}

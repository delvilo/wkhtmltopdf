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

#ifndef WKHTMLTOPDF_IMAGEOUTPUT_HH
#define WKHTMLTOPDF_IMAGEOUTPUT_HH

#include <QBuffer>
#include <QFile>
#include <QSaveFile>

#include "dllbegin.inc"
namespace wkhtmltopdf {

// File output is committed only after rendering and encoding have succeeded.
class DLL_LOCAL ImageOutput {
public:
	ImageOutput(const QString & path, QByteArray & data);
	bool open();
	QIODevice * device();
	bool commit();
	QString errorString() const;
private:
	Q_DISABLE_COPY(ImageOutput)
	QString path;
	QString error;
	QBuffer buffer;
	QFile standardOutput;
	QSaveFile file;
	QIODevice * destination;
};

}
#include "dllend.inc"
#endif

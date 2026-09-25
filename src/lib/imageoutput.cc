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

#include "imageoutput.hh"
#include <QFileInfo>
#include <cstdio>

namespace wkhtmltopdf {

ImageOutput::ImageOutput(const QString & outputPath, QByteArray & data):
	path(outputPath), buffer(&data), destination(0) {}

bool ImageOutput::open() {
	if (path.isEmpty()) {
		destination = &buffer;
		if (buffer.open(QIODevice::WriteOnly | QIODevice::Truncate)) return true;
		error = buffer.errorString();
		return false;
	}
	if (path == "-") {
		destination = &standardOutput;
		if (standardOutput.open(stdout, QIODevice::WriteOnly)) return true;
		error = standardOutput.errorString();
		return false;
	}

	const QFileInfo target(path);
	if (target.exists() && !target.isFile()) {
		error = "Image output must be a regular file or stdout";
		return false;
	}
	file.setFileName(path);
	// Never fall back to truncating the destination when a temporary file fails.
	file.setDirectWriteFallback(false);
	if (!file.open(QIODevice::WriteOnly)) {
		error = file.errorString();
		return false;
	}
	destination = &file;
	return true;
}

QIODevice * ImageOutput::device() {
	return destination;
}

bool ImageOutput::commit() {
	if (destination == &buffer) {
		buffer.close();
		return true;
	}
	if (destination == &standardOutput) {
		if (standardOutput.flush() && standardOutput.error() == QFile::NoError) return true;
		error = standardOutput.errorString();
		return false;
	}
	if (file.commit()) return true;
	error = file.errorString();
	return false;
}

QString ImageOutput::errorString() const {
	return error;
}

}

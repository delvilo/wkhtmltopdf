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


#include "imagesettings.hh"
#include "reflect.hh"
#include <QFileInfo>
#include <QImageWriter>

#include "dllbegin.inc"
namespace wkhtmltopdf {
namespace settings {

template<>
struct DLL_LOCAL ReflectImpl<CropSettings>: public ReflectClass {
	ReflectImpl(CropSettings & c) {
		WKHTMLTOPDF_REFLECT(left);
		WKHTMLTOPDF_REFLECT(top);
		WKHTMLTOPDF_REFLECT(width);
		WKHTMLTOPDF_REFLECT(height);
	}
};

template<>
struct DLL_LOCAL ReflectImpl<ImageGlobal>: public ReflectClass {
	ReflectImpl(ImageGlobal & c) {
		WKHTMLTOPDF_REFLECT(screenWidth);
		WKHTMLTOPDF_REFLECT(screenHeight);
		WKHTMLTOPDF_REFLECT(logLevel);
		WKHTMLTOPDF_REFLECT(transparent);
		WKHTMLTOPDF_REFLECT(in);
		WKHTMLTOPDF_REFLECT(out);
		WKHTMLTOPDF_REFLECT(fmt);
		WKHTMLTOPDF_REFLECT(quality);
		WKHTMLTOPDF_REFLECT(loadGlobal);
		WKHTMLTOPDF_REFLECT(loadPage);
		WKHTMLTOPDF_REFLECT(smartWidth);
	}
};

CropSettings::CropSettings():
	left(-1),
	top(-1),
	width(-1),
	height(-1) {}

ImageGlobal::ImageGlobal():
	logLevel(Info),
	transparent(false),
	in(""),
	out(""),
	fmt(""),
	screenWidth(1024),
	screenHeight(0),
	quality(94),
	smartWidth(true) {}

QString ImageGlobal::outputFormat() const {
	if (!fmt.isEmpty()) return fmt.toLower();
	if (out.isEmpty() || out == "-") return "jpg";
	return QFileInfo(out).suffix().toLower();
}

bool ImageGlobal::validate(QString & error) const {
	error.clear();
	if (screenWidth <= 0)
		error = "Image width must be greater than zero";
	else if (screenHeight < 0)
		error = "Image height must be zero (automatic) or greater";
	else if (quality < -1 || quality > 100)
		error = "Image quality must be between 0 and 100, or -1 for the encoder default";
	else if (crop.left < -1 || crop.top < -1)
		error = "Crop offsets must be non-negative, or -1 for the default";
	else if (crop.width < -1 || crop.height < -1 || crop.width == 0 || crop.height == 0)
		error = "Crop dimensions must be greater than zero, or -1 for the remaining image";
	if (!error.isEmpty()) return false;

	const QString format = outputFormat();
	if (format.isEmpty())
		error = "Cannot determine the output image format; specify --format";
	else if (format != "svg" && !QImageWriter::supportedImageFormats().contains(format.toLatin1()))
		error = QString("Unsupported output image format: %1").arg(format);
	return error.isEmpty();
}

QString ImageGlobal::get(const char * name) {
	ReflectImpl<ImageGlobal> impl(*this);
	return impl.get(name);
}

bool ImageGlobal::set(const char * name, const QString & value) {
	ReflectImpl<ImageGlobal> impl(*this);
	return impl.set(name, value);
}


}
}

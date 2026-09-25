# Copyright 2010-2020 wkhtmltopdf authors
#
# This file is part of wkhtmltopdf.
#
# wkhtmltopdf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# wkhtmltopdf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with wkhtmltopdf.  If not, see <http:#www.gnu.org/licenses/>.

!linux: error("Linux is the only supported platform")

!equals(QT_MAJOR_VERSION, 5): error("Qt5 with WebKit is required")
lessThan(QT_MINOR_VERSION, 3): error("Qt 5.3 or newer is required")

INCLUDEPATH += ../../src/lib
RESOURCES    = $$PWD/wkhtmltopdf.qrc

QT += webkit webkitwidgets network svg printsupport

# version related information
VERSION_TEXT=$$(WKHTMLTOX_VERSION)
isEmpty(VERSION_TEXT): VERSION_TEXT=0.12.7
VERSION_LIST=$$split(VERSION_TEXT, "-")

count(VERSION_LIST, 1): VERSION=$$VERSION_TEXT
else:                   VERSION=$$member(VERSION_LIST, 0)

DEFINES += VERSION=$$VERSION FULL_VERSION=$$VERSION_TEXT BUILDING_WKHTMLTOX

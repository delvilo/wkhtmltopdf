// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.
#include "renderbackend.hh"
#include "multipageloader.hh"

namespace wkhtmltopdf {

ResourceLoader * createResourceLoader(settings::LoadGlobal & settings) {
	return new MultiPageLoader(settings);
}

}

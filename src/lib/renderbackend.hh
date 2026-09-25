// Backend selection is centralized here; WebKit remains the only backend.
// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.

#ifndef WKHTMLTOPDF_RENDERBACKEND_HH
#define WKHTMLTOPDF_RENDERBACKEND_HH

#include "resourceloader.hh"

#include "dllbegin.inc"
namespace wkhtmltopdf {

// Caller owns the returned loader. No Qt6/WebEngine implementation is selected.
DLL_LOCAL ResourceLoader * createResourceLoader(settings::LoadGlobal & settings, int dpi, bool mainLoader = false);

}
#include "dllend.inc"
#endif

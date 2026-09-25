// Copyright 2026 wkhtmltopdf contributors. LGPL-3.0-or-later.
#ifndef WKHTMLTOPDF_WEBKITFEATURES_HH
#define WKHTMLTOPDF_WEBKITFEATURES_HH

// Patched Qt defines __EXTENSIVE_WKHTMLTOPDF_QT_HACK__ in qwebframe.h,
// not in Qt's global configuration. Import it before any feature guards,
// even when callers use only the rendering interfaces.
#include <QWebFrame>

#endif

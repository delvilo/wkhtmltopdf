#!/usr/bin/env python3
"""Exercise the reduced CLI and C API using freshly built binaries."""

import argparse
import ctypes
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest


SHARED_REMOVED = {
    'quiet', 'readme', 'htmldoc', 'enable-plugins', 'disable-plugins',
    'use-xserver', 'checkbox-svg', 'checkbox-checked-svg',
    'radiobutton-svg', 'radiobutton-checked-svg',
}
PDF_REMOVED = SHARED_REMOVED | {
    'default-header', 'lowquality', 'copies', 'collate', 'no-collate',
    'no-pdf-compression', 'enable-forms', 'disable-forms',
    'read-args-from-stdin', 'dump-outline', 'dump-default-toc-xsl',
}
HTML = b'''<!doctype html><html><head><meta charset="utf-8"></head>
<body><h1>Conversion smoke test</h1><p>Text remains visible.</p>
<input value="Static form value"><input type="checkbox" checked>
<input type="radio" checked></body></html>'''


class OptionRemovalSmoke(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wkhtmltox-smoke-')
        cls.work = Path(cls.temp.name)
        cls.source = cls.work / 'input.html'
        cls.source.write_bytes(HTML)
        cls.env = os.environ.copy()
        cls.env.setdefault('QT_QPA_PLATFORM', 'offscreen')
        result = cls.run_cli('wkhtmltopdf', '--version')
        cls.patched = b'with patched qt' in result.stdout

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    @classmethod
    def run_cli(cls, name, *args, data=None):
        executable = BIN_DIR / name
        return subprocess.run(
            [str(executable), *map(str, args)], input=data,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            env=cls.env, timeout=40,
        )

    def assert_success(self, result):
        self.assertEqual(result.returncode, 0, result.stderr.decode(errors='replace'))

    def assert_pdf(self, data, pages=None):
        self.assertTrue(data.startswith(b'%PDF-'), data[:120])
        self.assertIn(b'%%EOF', data[-100:])
        if pages is not None:
            self.assertEqual(len(re.findall(rb'/Type\s*/Page(?=[\s/>])', data)), pages)
        self.assertNotIn(b'/AcroForm', data)

    def test_removed_cli_options_and_short_aliases(self):
        for name, options in [('wkhtmltopdf', PDF_REMOVED), ('wkhtmltoimage', SHARED_REMOVED)]:
            for option in sorted(options):
                with self.subTest(program=name, option=option):
                    result = self.run_cli(name, '--' + option)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn(('Unknown long argument --' + option).encode(), result.stderr)
            aliases = ['-q', '-l'] if name == 'wkhtmltopdf' else ['-q']
            for alias in aliases:
                with self.subTest(program=name, alias=alias):
                    result = self.run_cli(name, alias)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn(b'Unknown switch', result.stderr)

    def test_help_and_manpage(self):
        for name, options in [('wkhtmltopdf', PDF_REMOVED), ('wkhtmltoimage', SHARED_REMOVED)]:
            for switch in ['--extended-help', '--manpage']:
                with self.subTest(program=name, switch=switch):
                    result = self.run_cli(name, switch)
                    self.assert_success(result)
                    output = (result.stdout + result.stderr).replace(b'\\-', b'-')
                    advertised = set(re.findall(rb'--([a-z][a-z-]*)', output))
                    self.assertFalse(advertised.intersection(option.encode() for option in options))
                    self.assertIn(b'log-level', advertised)

    def test_error_handling_values(self):
        for name in ['wkhtmltopdf', 'wkhtmltoimage']:
            for value in ['abort', 'ignore']:
                with self.subTest(program=name, policy=value):
                    self.assert_success(self.run_cli(name, '--load-media-error-handling', value, '--version'))
            result = self.run_cli(name, '--load-media-error-handling', 'skip', '--version')
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(b'Invalid argument(s)', result.stderr)
            self.assert_success(self.run_cli(name, '--load-error-handling', 'skip', '--version'))

    def test_pdf_file_and_stdin_stdout(self):
        target = self.work / 'output.pdf'
        self.assert_success(self.run_cli('wkhtmltopdf', '--log-level', 'none', self.source, target))
        self.assert_pdf(target.read_bytes(), pages=1)
        result = self.run_cli('wkhtmltopdf', '--log-level', 'none', '-', '-', data=HTML)
        self.assert_success(result)
        self.assert_pdf(result.stdout, pages=1)

    def test_image_stdin_stdout_and_crop(self):
        result = self.run_cli(
            'wkhtmltoimage', '--log-level', 'none', '--format', 'png',
            '--width', '320', '--height', '180', '--crop-w', '160', '--crop-h', '90',
            '-', '-', data=HTML,
        )
        self.assert_success(result)
        self.assertEqual(result.stdout[:8], b'\x89PNG\r\n\x1a\n')
        self.assertEqual(struct.unpack('>II', result.stdout[16:24]), (160, 90))

    def test_c_api_settings_and_conversion(self):
        path = BIN_DIR / 'libwkhtmltox.so'
        self.assertTrue(path.exists(), 'Build the shared library before running the smoke checks')
        lib = ctypes.CDLL(str(path))
        ptr = ctypes.c_void_p
        text = ctypes.c_char_p

        def bind(name, args, result):
            fn = getattr(lib, name)
            fn.argtypes, fn.restype = args, result
            return fn

        initialize = bind('wkhtmltopdf_init', [ctypes.c_int], ctypes.c_int)
        deinitialize = bind('wkhtmltopdf_deinit', [], ctypes.c_int)
        self.assertEqual(initialize(0), 1)
        try:
            for prefix, removed in [
                ('wkhtmltopdf', ['quiet', 'useGraphics', 'resolution', 'copies', 'collate', 'dumpOutline', 'useCompression']),
                ('wkhtmltoimage', ['quiet', 'useGraphics', 'loadPage.checkboxSvg', 'loadPage.checkboxCheckedSvg', 'loadPage.radiobuttonSvg', 'loadPage.radiobuttonCheckedSvg']),
            ]:
                create = bind(prefix + '_create_global_settings', [], ptr)
                setting = bind(prefix + '_set_global_setting', [ptr, text, text], ctypes.c_int)
                get = bind(prefix + '_get_global_setting', [ptr, text, ptr, ctypes.c_int], ctypes.c_int)
                destroy = bind(prefix + '_destroy_global_settings', [ptr], None)
                gs = create()
                try:
                    for key in removed:
                        with self.subTest(api=prefix, removed=key):
                            value = ctypes.create_string_buffer(100)
                            self.assertEqual(get(gs, key.encode(), value, len(value)), 0)
                            self.assertEqual(setting(gs, key.encode(), b'true'), 0)
                    self.assertEqual(setting(gs, b'logLevel', b'none'), 1)
                    value = ctypes.create_string_buffer(100)
                    self.assertEqual(get(gs, b'logLevel', value, len(value)), 1)
                    self.assertEqual(value.value, b'none')
                finally:
                    destroy(gs)

            create_object = bind('wkhtmltopdf_create_object_settings', [], ptr)
            set_object = bind('wkhtmltopdf_set_object_setting', [ptr, text, text], ctypes.c_int)
            destroy_object = bind('wkhtmltopdf_destroy_object_settings', [ptr], None)
            obj = create_object()
            try:
                for key in ['produceForms', 'web.enablePlugins', 'load.checkboxSvg', 'load.checkboxCheckedSvg', 'load.radiobuttonSvg', 'load.radiobuttonCheckedSvg']:
                    self.assertEqual(set_object(obj, key.encode(), b'true'), 0, key)
                self.assertEqual(set_object(obj, b'web.enableJavascript', b'false'), 1)
                self.assertEqual(set_object(obj, b'load.loadErrorHandling', b'skip'), 1)
            finally:
                destroy_object(obj)

            gs = lib.wkhtmltopdf_create_global_settings()
            self.assertEqual(lib.wkhtmltopdf_set_global_setting(gs, b'logLevel', b'none'), 1)
            converter = bind('wkhtmltopdf_create_converter', [ptr], ptr)(gs)
            obj = create_object()
            bind('wkhtmltopdf_add_object', [ptr, ptr, text], None)(converter, obj, HTML)
            try:
                self.assertEqual(bind('wkhtmltopdf_convert', [ptr], ctypes.c_int)(converter), 1)
                output = ptr()
                length = bind('wkhtmltopdf_get_output', [ptr, ctypes.POINTER(ptr)], ctypes.c_long)(converter, ctypes.byref(output))
                self.assertGreater(length, 0)
                self.assert_pdf(ctypes.string_at(output, length), pages=1)
            finally:
                bind('wkhtmltopdf_destroy_converter', [ptr], None)(converter)
        finally:
            deinitialize()

    def test_patched_qt_multi_object_headers_and_toc(self):
        if not self.patched:
            self.skipTest('Requires a build with patched Qt')
        header = self.work / 'header.html'
        header.write_text('<html><body>Header text</body></html>')
        target = self.work / 'book.pdf'
        result = self.run_cli(
            'wkhtmltopdf', '--log-level', 'none', '--enable-local-file-access',
            '--header-html', header, '--footer-center', '[page]/[topage]',
            'toc', self.source, self.source, target,
        )
        self.assert_success(result)
        self.assert_pdf(target.read_bytes(), pages=3)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bin-dir', type=Path, default=Path(__file__).resolve().parents[1] / 'bin')
    args = parser.parse_args()
    BIN_DIR = args.bin_dir.resolve()
    unittest.main(argv=[__file__], verbosity=2)

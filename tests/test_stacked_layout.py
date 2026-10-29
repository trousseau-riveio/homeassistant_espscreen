"""A single card's icon over its name and state ("layout": "stacked", local fork): the add-on keeps only "stacked",
the editor maps and checks it, and the firmware reads it and draws it."""
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'screen_manager/app'))
import core  # noqa: E402
import page_layout  # noqa: E402

COMPONENT = ROOT / 'components/smart_display'
RECEIVER = (COMPONENT / 'page_receiver.cpp').read_text()
MODEL = (COMPONENT / 'runtime_model.h').read_text()
TILES = (COMPONENT / 'runtime_tiles.h').read_text()
OPTIONS = (ROOT / 'web/src/model/tile-options.ts').read_text()
VALIDATION = (ROOT / 'web/src/model/page-validation.ts').read_text()


def layout(entity, **options):
    return {'title': 'Home', 'tiles': [{'entity': entity, 'slot': 0, **({'options': options} if options else {})}]}


class TheApp(unittest.TestCase):
    def test_stacked_is_kept_and_the_default_is_dropped(self):
        self.assertEqual(core.validate_layout(layout('lock.door', layout='stacked'))['tiles'][0]['options'], {'layout': 'stacked'})
        self.assertNotIn('layout', core.validate_layout(layout('lock.door', layout='row'))['tiles'][0].get('options', {}))

    def test_another_word_is_refused(self):
        with self.assertRaises(ValueError):
            core.validate_layout(layout('lock.door', layout='sideways'))

    def test_the_page_format_carries_it_as_the_tile_option(self):
        self.assertEqual(page_layout.APPEARANCE['layout'], 'layout')


class TheEditor(unittest.TestCase):
    def test_it_maps_drops_the_default_and_checks_it(self):
        self.assertIn('layout: "layout"', OPTIONS)
        self.assertIn('if (out.layout !== "stacked") delete out.layout;', OPTIONS)
        self.assertIn("a.layout !== 'stacked'", VALIDATION)


class TheFirmware(unittest.TestCase):
    def test_it_reads_and_draws_it(self):
        self.assertIn('bool stacked = false;', MODEL)
        self.assertIn('tile.stacked = string(options["layout"], 8) == "stacked";', RECEIVER)
        self.assertIn('t.stacked && !custom && !watch && !mini && !graph && !with_panel && !w.wide && !w.full', TILES)


if __name__ == '__main__':
    unittest.main()

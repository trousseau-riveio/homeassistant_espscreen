"""Render the Soft look (local fork, theme 3) with the real firmware on the host (tools/render/host.py): every card size
of a 3 x 3 grid, lamps on and off, with a switch or a slider, a lock, a sensor, a blind and a lamp that is unavailable.

    ESPHOME=<esphome> python3 tools/render/soft_study.py [guition]

The look is chosen through the core's Theme select, and its colours through Soft colours. Shots land in .esphome/soft-study/out/<variant>/<page>.png.
"""
import argparse
import asyncio
import os
import shlex
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import run  # noqa: E402
import host  # noqa: E402
import send_layout  # noqa: E402
import page_layout  # noqa: E402

PALETTES = ('Warm', 'Sage', 'Ocean', 'Rose', 'Lavender', 'Sand', 'Stone',
            'Warm Dark', 'Sage Dark', 'Ocean Dark', 'Rose Dark', 'Lavender Dark', 'Sand Dark', 'Graphite')
NAMES = {(1, 1): 'single', (2, 1): 'wide', (1, 2): 'tall', (2, 2): 'square'}
STATES = {
    'light.on': {'state': 'on', 'attributes': {'brightness': 250, 'supported_color_modes': ['brightness'], 'color_mode': 'brightness'}},
    'light.dim': {'state': 'on', 'attributes': {'brightness': 46, 'supported_color_modes': ['brightness'], 'color_mode': 'brightness'}},
    'light.off': {'state': 'off', 'attributes': {'supported_color_modes': ['brightness']}},
    'light.gone': {'state': 'unavailable', 'attributes': {}},
    'switch.on': {'state': 'on', 'attributes': {}},
    'switch.off': {'state': 'off', 'attributes': {}},
    'lock.door': {'state': 'locked', 'attributes': {}},
    'sensor.temp': {'state': '21.4', 'attributes': {'unit_of_measurement': '°C', 'device_class': 'temperature'}},
    'cover.blind': {'state': 'open', 'attributes': {'current_position': 60, 'supported_features': 15}},
    'sensor.temp_f': {'state': '72.5', 'attributes': {'unit_of_measurement': '°F', 'device_class': 'temperature'}},
    'sensor.humidity': {'state': '47', 'attributes': {'unit_of_measurement': '%', 'device_class': 'humidity'}},
    'sensor.power': {'state': '1840', 'attributes': {'unit_of_measurement': 'W', 'device_class': 'power'}},
    'sensor.co2': {'state': '842', 'attributes': {'unit_of_measurement': 'ppm', 'device_class': 'carbon_dioxide'}},
    'cover.garage': {'state': 'closed', 'attributes': {'current_position': 0, 'device_class': 'garage', 'supported_features': 11}},
    'cover.shutter': {'state': 'open', 'attributes': {'current_position': 35, 'current_tilt_position': 50, 'supported_features': 255}},
    'climate.ac': {'state': 'cool', 'attributes': {'current_temperature': 21.5, 'temperature': 20, 'min_temp': 16, 'max_temp': 32, 'target_temp_step': 1.0,
                                                   'hvac_modes': ['off', 'heat_cool', 'cool', 'heat', 'fan_only', 'dry'], 'supported_features': 425}},
    'climate.heating': {'state': 'heat', 'attributes': {'current_temperature': 19.5, 'temperature': 21, 'hvac_action': 'heating', 'hvac_modes': ['off', 'heat', 'auto'],
                                                        'supported_features': 1}},
    'media_player.sonos': {'state': 'playing', 'attributes': {'volume_level': 0.17, 'is_volume_muted': False, 'media_title': 'Morning jazz', 'supported_features': 8321599}},
    'vacuum.robot': {'state': 'docked', 'attributes': {'battery_level': 100, 'fan_speed': 'max', 'fan_speed_list': ['quiet', 'max'], 'supported_features': 30524}},
    'number.target': {'state': '55', 'attributes': {'min': 30, 'max': 70, 'step': 5, 'unit_of_measurement': '%'}},
    'select.mode': {'state': 'Comfort', 'attributes': {'options': ['Eco', 'Comfort', 'Boost']}},
    'scene.evening': {'state': '2026-10-28T09:00:00+00:00', 'attributes': {}},
    'script.tv': {'state': 'off', 'attributes': {'last_triggered': '2026-10-28T08:00:00+00:00'}},
    'fan.fan': {'state': 'on', 'attributes': {'percentage': 66, 'percentage_step': 33.3, 'supported_features': 1}},
    'humidifier.bath': {'state': 'on', 'attributes': {'humidity': 50, 'current_humidity': 44, 'min_humidity': 30, 'max_humidity': 80}},
    'automation.night': {'state': 'on', 'attributes': {'last_triggered': '2026-10-28T08:00:00+00:00'}},
    'timer.eggs': {'state': 'active', 'attributes': {'duration': '0:05:00', 'remaining': '0:04:32', 'finishes_at': '2026-10-28T12:04:32+00:00'}},
    'sensor.graph': {'state': '21.4', 'attributes': {'unit_of_measurement': '°C', 'device_class': 'temperature'}},
}
HISTORY = {'hours': 24, 'values': [17.8, 17.2, 16.9, 16.8, 17.4, 18.6, 19.9, 20.8, 21.6, 22.4, 23.1, 23.4,
                                   23.2, 22.9, 22.1, 21.4, 20.9, 20.6, 20.1, 19.8, 20.4, 21.0, 21.3, 21.4]}
WATCH = {'display': 'watch'}
CHOSEN = ('garage', 'blinds', 'lock', 'gate')  # icons sent as chosen in the editor, to see them follow the state
GRAPH = {'display': 'graph'}
TOGGLE, SLIDER = {}, {'controls': 'brightness'}
# (page, [(entity, name, icon, options, column, row, columns, rows)])
PAGES = [
    ('singles', [('light.on', 'Desk lamp', 'lamp', TOGGLE, 0, 0, 1, 1), ('light.off', 'Ceiling', 'ceiling-light', TOGGLE, 1, 0, 1, 1),
                 ('lock.door', 'Front door', 'lock', {}, 2, 0, 1, 1), ('switch.off', 'Sprinklers', 'sprinkler', {}, 0, 1, 1, 1),
                 ('sensor.temp', 'Living room', 'thermometer', {}, 1, 1, 1, 1), ('light.dim', 'Night light', 'lamp', {'inline': 'slider'}, 2, 1, 1, 1),
                 ('cover.blind', 'Blind', 'blinds', {}, 0, 2, 1, 1), ('light.gone', 'Garden', 'outdoor-lamp', TOGGLE, 1, 2, 1, 1),
                 ('switch.on', 'Fountain', 'fountain', {}, 2, 2, 1, 1)]),
    ('strips', [('light.on', 'Shelf', 'lightbulb', TOGGLE, 0, 0, 2, 1), ('light.off', 'Kitchen', 'ceiling-light', TOGGLE, 2, 0, 1, 1),
                ('light.dim', 'Shelf dimmer', 'lightbulb', SLIDER, 0, 1, 3, 1), ('switch.off', 'Outdoor lights', 'outdoor-lamp', {}, 0, 2, 3, 1)]),
    ('talls', [('light.on', 'Floor lamp', 'floor-lamp', SLIDER, 0, 0, 1, 2), ('light.off', 'Reading', 'lamp', TOGGLE, 1, 0, 1, 2),
               ('light.dim', 'Hallway', 'ceiling-light', SLIDER, 2, 0, 1, 3), ('cover.blind', 'Garage', 'garage', {}, 0, 2, 2, 1)]),
    ('squares', [('light.on', 'Lamp', 'floor-lamp', TOGGLE, 0, 0, 2, 2), ('light.dim', 'Bedroom', 'bed', SLIDER, 2, 0, 1, 3),
                 ('switch.on', 'Coffee machine', 'coffee-maker', {}, 0, 2, 2, 1)]),
    ('big', [('light.on', 'Living room', 'sofa', SLIDER, 0, 0, 2, 3), ('light.off', 'Porch', 'outdoor-lamp', TOGGLE, 2, 0, 1, 3)]),
    ('wide', [('light.dim', 'Dining table', 'table-furniture', SLIDER, 0, 0, 3, 2), ('light.on', 'Office', 'desk-lamp', TOGGLE, 0, 2, 3, 1)]),
    ('full', [('light.on', 'Whole room', 'home-lightbulb', TOGGLE, 0, 0, 3, 3)]),
    ('sensors', [('sensor.temp_f', 'Outside temperature', '', WATCH, 0, 0, 1, 1), ('sensor.humidity', 'Humidity', '', WATCH, 1, 0, 1, 1),
                 ('sensor.power', 'House power', '', WATCH, 2, 0, 1, 2), ('sensor.co2', 'Office CO2', '', WATCH, 0, 1, 2, 1),
                 ('sensor.temp_f', 'Living room', '', WATCH, 0, 2, 3, 1)]),
    ('sensors_big', [('sensor.temp_f', 'Outside temperature', '', WATCH, 0, 0, 2, 2), ('sensor.power', 'House power', '', WATCH, 2, 0, 1, 3),
                     ('sensor.humidity', 'Bathroom', '', WATCH, 0, 2, 2, 1)]),
    ('graphs', [('sensor.graph', 'Living room', '', GRAPH, 0, 0, 1, 1), ('sensor.graph', 'Outside', '', GRAPH, 1, 0, 2, 1),
                ('sensor.graph', 'Bedroom', '', GRAPH, 0, 1, 1, 2), ('sensor.graph', 'Office', '', GRAPH, 1, 1, 2, 2)]),
    ('graphs_wide', [('sensor.graph', 'Outside temperature', '', GRAPH, 0, 0, 3, 1), ('sensor.graph', 'Garden', '', GRAPH, 0, 1, 3, 2)]),
    ('graphs_tall', [('sensor.graph', 'Kitchen', '', GRAPH, 0, 0, 2, 3), ('sensor.graph', 'Hall', '', GRAPH, 2, 0, 1, 3)]),
    ('c_cover', [('cover.shutter', 'Living blinds', '', {'controls': 'buttons'}, 0, 0, 2, 1), ('cover.garage', 'Garage', 'garage', {'controls': 'buttons'}, 2, 0, 1, 1),
                 ('cover.shutter', 'Bedroom blinds', '', {'controls': 'buttons'}, 0, 1, 2, 2), ('cover.garage', 'Gate', 'gate', {'controls': 'buttons'}, 2, 1, 1, 2)]),
    ('c_cover2', [('cover.shutter', 'Kitchen blinds', '', {'controls': 'position'}, 0, 0, 2, 1), ('cover.shutter', 'Office blinds', '', {'controls': 'buttons_tilt'}, 0, 1, 3, 1),
                  ('cover.shutter', 'Tilt', '', {'controls': 'tilt'}, 2, 0, 1, 1), ('cover.shutter', 'Shutter', '', {'controls': 'position_tilt'}, 0, 2, 3, 1)]),
    ('c_cover3', [('cover.shutter', 'Blind', '', {'controls': 'position'}, 0, 0, 1, 1), ('cover.shutter', 'Shutter', '', {'controls': 'buttons'}, 1, 0, 1, 1),
                  ('cover.shutter', 'Slats', '', {'controls': 'tilt'}, 2, 0, 1, 1), ('cover.shutter', 'Office blinds', '', {'controls': 'buttons_tilt'}, 0, 1, 2, 2),
                  ('cover.garage', 'Garage', 'garage', {'controls': 'buttons'}, 2, 1, 1, 1), ('cover.shutter', 'Kitchen', '', {'controls': 'position_tilt'}, 2, 2, 1, 1)]),
    ('c_climate', [('climate.ac', 'Air conditioning', '', {'controls': 'setpoint'}, 0, 0, 2, 1), ('climate.heating', 'Heating', '', {'controls': 'mode'}, 0, 1, 3, 1),
                   ('climate.ac', 'Bedroom', '', {'controls': 'setpoint'}, 2, 0, 1, 1), ('climate.heating', 'Office heat', '', {'controls': 'setpoint_mode'}, 0, 2, 3, 1)]),
    ('c_climate2', [('climate.ac', 'Air conditioning', '', {'controls': 'setpoint_mode'}, 0, 0, 2, 2), ('climate.heating', 'Heating', '', {'controls': 'setpoint'}, 2, 0, 1, 3),
                    ('climate.heating', 'Hall', '', {'controls': 'mode'}, 0, 2, 2, 1)]),
    ('c_media', [('media_player.sonos', 'Kitchen speaker', '', {'controls': 'playback'}, 0, 0, 3, 1), ('media_player.sonos', 'Radio', '', {'controls': 'volume'}, 0, 1, 2, 1),
                 ('media_player.sonos', 'Living room', '', {'controls': 'volume_playback'}, 2, 1, 1, 2), ('media_player.sonos', 'Office', '', {'controls': 'playback'}, 0, 2, 2, 1)]),
    ('c_media2', [('media_player.sonos', 'Living room', '', {'controls': 'volume_playback'}, 0, 0, 2, 2), ('media_player.sonos', 'Bedroom', '', {'controls': 'playback'}, 2, 0, 1, 2),
                  ('media_player.sonos', 'Bath', '', {'controls': 'volume'}, 0, 2, 3, 1)]),
    ('c_misc', [('select.mode', 'Heating mode', '', {'controls': 'stepper'}, 0, 0, 2, 1), ('number.target', 'Target', '', {'controls': 'stepper'}, 0, 1, 2, 1),
                ('scene.evening', 'Evening', '', {'controls': 'run'}, 0, 2, 2, 1), ('timer.eggs', 'Eggs', '', {'controls': 'buttons'}, 2, 0, 1, 2),
                ('script.tv', 'TV on', '', {'controls': 'run'}, 2, 2, 1, 1)]),
    ('c_misc2', [('vacuum.robot', 'Robot', '', {'controls': 'buttons'}, 0, 0, 2, 1), ('humidifier.bath', 'Bath', '', {'controls': 'slider'}, 0, 1, 2, 1),
                 ('fan.fan', 'Ceiling fan', '', {'controls': 'speed'}, 0, 2, 2, 1), ('automation.night', 'Night mode', '', {'controls': 'run'}, 2, 0, 1, 2),
                 ('number.target', 'Level', '', {'controls': 'slider'}, 2, 2, 1, 1)]),
    ('c_misc3', [('vacuum.robot', 'Robot', '', {'controls': 'buttons'}, 0, 0, 2, 2), ('select.mode', 'Mode', '', {'controls': 'stepper'}, 2, 0, 1, 2),
                 ('humidifier.bath', 'Humidifier', '', {'controls': 'toggle'}, 0, 2, 2, 1), ('automation.night', 'Lights', '', {'controls': 'toggle'}, 2, 2, 1, 1)]),
    ('c_full', [('climate.ac', 'Air conditioning', '', {'controls': 'setpoint_mode'}, 0, 0, 3, 3)]),
    ('c_stacked', [('light.on', 'Desk lamp', 'lamp', {'layout': 'stacked'}, 0, 0, 1, 1), ('light.off', 'Ceiling', 'ceiling-light', {'layout': 'stacked'}, 1, 0, 1, 1),
                   ('sensor.temp', 'Living room', 'thermometer', {'layout': 'stacked'}, 2, 0, 1, 1), ('lock.door', 'Front door', 'lock', {'layout': 'stacked'}, 0, 1, 1, 1),
                   ('cover.garage', 'Garage', 'garage', {'controls': 'buttons'}, 1, 1, 1, 1), ('cover.shutter', 'Shutter', '', {'controls': 'buttons'}, 2, 1, 1, 1),
                   ('select.mode', 'Mode', '', {'controls': 'stepper'}, 0, 2, 1, 1), ('timer.eggs', 'Eggs', '', {'controls': 'buttons'}, 1, 2, 1, 1),
                   ('media_player.sonos', 'Radio', '', {'controls': 'playback'}, 2, 2, 1, 1)]),
    ('c_media_full', [('media_player.sonos', 'Living room', '', {'controls': 'volume_playback'}, 0, 0, 3, 3)]),
    ('graph_full', [('sensor.graph', 'Outside temperature', '', GRAPH, 0, 0, 3, 3)]),
]


class Study(run.Run):
    def __init__(self, build, out, pictures):
        super().__init__(build, out, pictures, (1280, 720))

    async def drive(self):
        self.client = run.APIClient('127.0.0.1', self.item.port, None)
        for _ in range(240):
            if self.process.poll() is not None:
                raise RuntimeError(f'the program exited with {self.process.returncode}')
            try:
                await self.client.connect(login=True)
                break
            except Exception:
                await asyncio.sleep(0.5)
        entities, services = await self.client.list_entities_services()
        self.services = {s.name: s for s in services}
        self.inbox = next(e for e in entities if type(e).__name__ == 'TextInfo' and e.name == 'Tile settings')
        theme = next(e for e in entities if getattr(e, 'name', '') == 'Theme')
        top_bar = next((e for e in entities if getattr(e, 'name', '') == 'Top bar'), None)
        self.subscribe_logs()
        if 'render_skip_calibration' in self.services:
            await self.call('render_skip_calibration')
        await self.call('render_time', epoch=int(run.MOMENT.timestamp()))
        grid = send_layout.Grid(3, 3)
        self.sender = send_layout.api_sender(self.client, services)
        inner = self.sender.send
        async def logged(message):
            answer = await inner(message)
            print('->', str(message)[:160], '\n<-', str(answer)[:300], flush=True)
            return answer
        self.sender.send = logged
        region = dict(keepalive=120, clock_24h=True, numbers='point', group_min=1, percent_space=False)
        states, pages = {}, []
        for page, (title, cells) in enumerate(PAGES):
            tiles = []
            for n, (state, name, icon, options, c, r, w, h) in enumerate(cells):
                domain = state.split('.')[0]
                entity = f'{domain}.{state.split(".")[1]}_p{page}_{n}'
                states[entity] = STATES[state]
                tiles.append({'id': f'a{page:03x}{n:012x}', 'content': {'kind': 'entity', 'entityId': entity},
                              'placement': {'row': r, 'column': c, 'columns': w, 'rows': h},
                              'appearance': {'label': name, 'presentation': page_layout.footprint_size(w, h, grid),
                                             **({'icon': icon} if icon in CHOSEN else {}),
                                             **{k: v for k, v in options.items() if k in ('display', 'layout')}},
                              'interaction': {k: v for k, v in options.items() if k not in ('display', 'layout')}})
            pages.append({'id': f'b{page:015x}', 'navigation': {'excludeFromPagination': False},
                          'topbar': {'leading': [], 'title': {'source': 'text', 'text': title}, 'trailing': []}, 'tiles': tiles})
        record = {'format': 'pages-v2', 'sourceGrid': {'columns': 3, 'rows': 3},
                  'layout': {'title': 'Soft', 'homePageId': pages[0]['id'], 'pages': pages}}
        compiled = send_layout.compile_tiles(record['layout'], grid)
        values = [send_layout.state_message(i, tile, states) for i, tile in enumerate(compiled)]
        for message, tile in zip(values, compiled):
            if tile['entity'].startswith('sensor.graph'):
                message['history'] = HISTORY
        bars = [[{'k': 'clock'}] for _ in record['layout']['pages']]
        await self.sender.synchronize(self.inbox.object_id, record, region, values, bars)
        self.client.select_command(theme.key, 'Soft')
        if top_bar:
            self.client.switch_command(top_bar.key, False)
        colours = next((e for e in entities if getattr(e, 'name', '') == 'Soft colours'), None)
        await asyncio.sleep(1.5)
        if PALETTES and colours:
            # Soft's colours: one page in each palette.
            for palette in PALETTES:
                self.client.select_command(colours.key, palette)
                await asyncio.sleep(1.0)
                await self.call('render_page', page=0)
                await self.page_done(0)
                await asyncio.sleep(0.5)
                await self.render(f'palette_{palette.lower().replace(" ", "_")}', timeout=6)
            self.client.select_command(colours.key, 'Warm')
            await asyncio.sleep(1.0)
        for page, (name, _) in enumerate(PAGES):
            await self.call('render_page', page=page)
            await self.page_done(page)
            await asyncio.sleep(0.5)
            await self.render(name, timeout=6)
        return len(PAGES), len(PAGES)



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('variants', nargs='*', default=['guition'])
    parser.add_argument('--out', type=Path, default=run.REPO / '.esphome' / 'soft-study' / 'out')
    parser.add_argument('--pages', help='only these pages, by name, comma-separated (a screen holds 24); no palettes then')
    args = parser.parse_args()
    if args.pages:
        wanted = args.pages.split(',')
        PAGES[:] = [page for page in PAGES if page[0] in wanted]
        globals()['PALETTES'] = ()
    esphome = shlex.split(os.environ.get('ESPHOME', 'esphome'))
    pictures = run.Pictures()
    for key in args.variants:
        item = host.variant(key)
        build = host.Build(item, tree=run.REPO, work=None, esphome=esphome)
        started = time.monotonic()
        ok, output = build.compile()
        if not ok:
            print(f'{key}: BUILD FAILED\n{output[-6000:]}', flush=True)
            continue
        study = Study(build, args.out.resolve() / key, pictures)
        try:
            summary = asyncio.run(study.run())
        except Exception as error:
            summary = f'stopped: {type(error).__name__}: {error}'
        print(f'{key}: {summary} ({time.monotonic() - started:.0f} s) {study.warnings}', flush=True)


if __name__ == '__main__':
    main()

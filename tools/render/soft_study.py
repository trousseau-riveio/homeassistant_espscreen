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
    'media_player.spotify': {'state': 'playing', 'attributes': {
        'volume_level': 0.42, 'is_volume_muted': False, 'media_title': 'Midnight City', 'media_artist': 'M83',
        'media_album_name': "Hurry Up, We're Dreaming", 'media_duration': 244, 'media_position': 71,
        'media_position_updated_at': '2026-10-28T10:00:00+00:00', 'entity_picture': '/api/media_player_proxy/media_player.spotify?cache=abc',
        'shuffle': False, 'repeat': 'off', 'supported_features': 8321599}},
    'media_player.paused': {'state': 'paused', 'attributes': {
        'volume_level': 0.3, 'media_title': 'Teardrop', 'media_artist': 'Massive Attack', 'media_album_name': 'Mezzanine',
        'media_duration': 330, 'media_position': 120, 'entity_picture': '/api/media_player_proxy/media_player.paused?cache=def',
        'supported_features': 8321599}},
    'media_player.long': {'state': 'playing', 'attributes': {
        'volume_level': 0.42, 'media_title': 'Symphony No. 9 in D minor, Op. 125: IV. Presto, Allegro assai',
        'media_artist': 'Berliner Philharmoniker, Herbert von Karajan', 'media_album_name': 'Beethoven: The Nine Symphonies',
        'media_duration': 1500, 'media_position': 71, 'supported_features': 8321599}},
    'media_player.idle': {'state': 'idle', 'attributes': {'volume_level': 0.3, 'supported_features': 8321599}},
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
    'sensor.energy': {'state': '12345.6', 'attributes': {'unit_of_measurement': 'kWh', 'device_class': 'energy'}},
    'sensor.pressure': {'state': '1016.4', 'attributes': {'unit_of_measurement': 'hPa', 'device_class': 'pressure'}},
    'sensor.text': {'state': 'Partly cloudy', 'attributes': {}},
    # The rest of the types, for the audit pages (a_*).
    'alarm_control_panel.home': {'state': 'armed_home', 'attributes': {'supported_features': 63, 'code_format': 'number', 'code_arm_required': True}},
    'alarm_control_panel.off': {'state': 'disarmed', 'attributes': {'supported_features': 63, 'code_format': 'number'}},
    'binary_sensor.door': {'state': 'on', 'attributes': {'device_class': 'door'}},
    'binary_sensor.motion': {'state': 'off', 'attributes': {'device_class': 'motion'}},
    'input_boolean.guest': {'state': 'on', 'attributes': {}},
    'input_number.volume': {'state': '40', 'attributes': {'min': 0, 'max': 100, 'step': 1, 'unit_of_measurement': '%'}},
    'input_select.scene': {'state': 'Movie', 'attributes': {'options': ['Day', 'Movie', 'Night']}},
    'button.reboot': {'state': '2026-10-28T07:00:00+00:00', 'attributes': {}},
    'remote.tv': {'state': 'on', 'attributes': {'current_activity': 'Watch TV', 'activity_list': ['Watch TV', 'Music']}},
    'person.tim': {'state': 'home', 'attributes': {}},
    'person.away': {'state': 'not_home', 'attributes': {}},
    'weather.home': {'state': 'partlycloudy', 'attributes': {'temperature': 18, 'humidity': 60, 'temperature_unit': '°C', 'wind_speed': 12,
                                                             'wind_speed_unit': 'km/h', 'pressure': 1016, 'supported_features': 3}},
    'sun.sun': {'state': 'above_horizon', 'attributes': {'next_rising': '2026-10-29T11:20:00+00:00', 'next_setting': '2026-10-28T22:05:00+00:00', 'elevation': 31}},
    'calendar.home': {'state': 'on', 'attributes': {'message': 'Dentist', 'start_time': '2026-10-28 14:00:00', 'end_time': '2026-10-28 15:00:00'}},
    'lock.open': {'state': 'unlocked', 'attributes': {}},
    'vacuum.cleaning': {'state': 'cleaning', 'attributes': {'battery_level': 64, 'fan_speed': 'max', 'fan_speed_list': ['quiet', 'max'], 'supported_features': 30524}},
}
# What the app works out for some types besides their state (core.extras): a forecast, the sun's times, a calendar's
# events (minutes from today 00:00, run.MOMENT's day).
_DAYS = [('Tue', 'partlycloudy', 19, 11, 20, 2), ('Wed', 'rainy', 15, 9, 80, 3), ('Thu', 'cloudy', 16, 10, 40, 4),
         ('Fri', 'sunny', 21, 12, 0, 5), ('Sat', 'sunny', 23, 13, 0, 6), ('Sun', 'partlycloudy', 20, 12, 10, 0), ('Mon', 'rainy', 14, 8, 90, 1)]
_HOURS = [(f'{h:02d}:00', c, t, r, h) for h, c, t, r in
          [(11, 'partlycloudy', 16, 10), (12, 'sunny', 17, 0), (13, 'sunny', 18, 0), (14, 'partlycloudy', 18, 10), (15, 'cloudy', 17, 30),
           (16, 'rainy', 15, 70), (17, 'rainy', 14, 80), (18, 'cloudy', 13, 20)]]
EXTRAS = {
    'weather.home': {'days': [{'d': d, 'c': c, 'h': h, 'l': l, 'p': p, 'w': w} for d, c, h, l, p, w in _DAYS],
                     'hours': [{'t': t, 'c': c, 'h': h, 'p': p, 'o': o} for t, c, h, p, o in _HOURS]},
    'sun.sun': {'rise': '07:20', 'set': '18:05'},
    # A sunny noon (energy_flow.py's form): solar 2.4 kW, 1.6 kW of it into the house, 300 W out to the grid, 500 W into
    # the battery at 72 %, and the devices drawing most.
    'screen.energy': {'h': 7, 'p': [2400, 0, 300, 0, 500, 1600], 'f': [1600, 300, 500, 0, 0, 0, 0], 'c': 72, 'n': 'Home',
                      'd': [{'n': 'Dishwasher', 'w': 640}, {'n': 'Fridge', 'w': 120}, {'n': 'Office', 'w': 310}], 'o': 530},
    'calendar.home': {'cal': {'d': [2026, 9, 15, 2], 'r': [0, 7], 'c': [[0xE53935, 'Family'], [0x1E88E5, 'Work']],
                              'e': [[600, 660, 0, 0, 'Dentist', 'Main Street'], [780, 840, 0, 1, 'Team call', ''],
                                    [1440 + 540, 1440 + 600, 0, 0, 'School run', ''], [2880 + 1140, 2880 + 1260, 0, 1, 'Dinner with Sam', 'Rue Verte']]}},
}
HISTORY = {'hours': 24, 'values': [17.8, 17.2, 16.9, 16.8, 17.4, 18.6, 19.9, 20.8, 21.6, 22.4, 23.1, 23.4,
                                   23.2, 22.9, 22.1, 21.4, 20.9, 20.6, 20.1, 19.8, 20.4, 21.0, 21.3, 21.4]}
WATCH = {'display': 'watch'}
CHOSEN = ('garage', 'blinds', 'lock', 'gate')  # icons sent as chosen in the editor, to see them follow the state
GRAPH = {'display': 'graph'}
TOGGLE, SLIDER = {}, {'controls': 'brightness'}
APPEARANCE = ('display', 'layout', 'subtitle')  # the options a tile's look carries; the rest are its interaction
# (page, [(entity, name, icon, options, column, row, columns, rows)])
PAGES = [
    ('singles', [('light.on', 'Desk lamp', 'lamp', TOGGLE, 0, 0, 1, 1), ('light.off', 'Ceiling', 'ceiling-light', TOGGLE, 1, 0, 1, 1),
                 ('lock.door', 'Front door', 'lock', {}, 2, 0, 1, 1), ('switch.off', 'Sprinklers', 'sprinkler', {}, 0, 1, 1, 1),
                 ('sensor.temp', 'Living room', 'thermometer', {}, 1, 1, 1, 1), ('light.dim', 'Night light', 'lamp', {'inline': 'slider'}, 2, 1, 1, 1),
                 ('cover.blind', 'Blind', 'blinds', {}, 0, 2, 1, 1), ('light.gone', 'Garden', 'outdoor-lamp', TOGGLE, 1, 2, 1, 1),
                 ('switch.on', 'Fountain', 'fountain', {}, 2, 2, 1, 1)]),
    ('strips', [('light.on', 'Shelf', 'lightbulb', TOGGLE, 0, 0, 2, 1), ('light.off', 'Kitchen', 'ceiling-light', TOGGLE, 2, 0, 1, 1),
                ('light.dim', 'Shelf dimmer', 'lightbulb', SLIDER, 0, 1, 3, 1), ('switch.off', 'Outdoor lights', 'outdoor-lamp', {}, 0, 2, 3, 1)]),
    ('strips_off', [('light.off', 'Office shelf', 'lightbulb', SLIDER, 0, 0, 2, 1), ('light.dim', 'Office shelf', 'lightbulb', SLIDER, 0, 1, 2, 1),
                    ('light.off', 'Office shelf', 'lightbulb', SLIDER, 0, 2, 3, 1), ('light.off', 'Lamp', 'lamp', SLIDER, 2, 0, 1, 2)]),
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
    ('m_mix', [('media_player.spotify', 'Spotify', '', {'controls': 'playback', 'display': 'cover'}, 0, 0, 2, 2),
               ('media_player.paused', 'Kitchen', '', {'controls': 'playback', 'display': 'cover'}, 2, 0, 1, 1),
               ('media_player.idle', 'Bedroom', '', {'controls': 'playback'}, 2, 1, 1, 1),
               ('media_player.spotify', 'Office', '', {'controls': 'volume_playback', 'display': 'cover'}, 0, 2, 3, 1)]),
    ('m_wide', [('media_player.spotify', 'Spotify', '', {'controls': 'volume_playback', 'display': 'cover'}, 0, 0, 3, 2),
                ('media_player.paused', 'Kitchen', '', {'controls': 'playback', 'display': 'cover'}, 0, 2, 2, 1),
                ('media_player.spotify', 'Bath', '', {'controls': 'volume', 'display': 'cover'}, 2, 2, 1, 1)]),
    ('m_tall', [('media_player.spotify', 'Spotify', '', {'controls': 'volume_playback', 'display': 'cover'}, 0, 0, 1, 3),
                ('media_player.paused', 'Kitchen', '', {'controls': 'volume_playback', 'display': 'cover'}, 1, 0, 2, 3)]),
    ('m_plain', [('media_player.spotify', 'Spotify', '', {'controls': 'playback'}, 0, 0, 2, 2),
                 ('media_player.paused', 'Kitchen', '', {'controls': 'playback'}, 2, 0, 1, 2),
                 ('media_player.spotify', 'Office', '', {'controls': 'volume'}, 0, 2, 2, 1),
                 ('media_player.idle', 'Bedroom', '', {'controls': 'playback'}, 2, 2, 1, 1)]),
    ('a_single', [('alarm_control_panel.home', 'Alarm', '', {}, 0, 0, 1, 1), ('binary_sensor.door', 'Front door', '', {}, 1, 0, 1, 1),
                  ('binary_sensor.motion', 'Hallway', '', {}, 2, 0, 1, 1), ('input_boolean.guest', 'Guest mode', '', {}, 0, 1, 1, 1),
                  ('button.reboot', 'Reboot', '', {}, 1, 1, 1, 1), ('remote.tv', 'TV', '', {}, 2, 1, 1, 1),
                  ('person.tim', 'Tim', '', {}, 0, 2, 1, 1), ('weather.home', 'Weather', '', {}, 1, 2, 1, 1), ('sun.sun', 'Sun', '', {}, 2, 2, 1, 1)]),
    ('a_watch', [('alarm_control_panel.off', 'Alarm', '', WATCH, 0, 0, 1, 1), ('binary_sensor.door', 'Front door', '', WATCH, 1, 0, 1, 1),
                 ('person.away', 'Tim', '', WATCH, 2, 0, 1, 1), ('lock.open', 'Back door', '', WATCH, 0, 1, 1, 1),
                 ('weather.home', 'Outside', '', WATCH, 1, 1, 1, 1), ('sun.sun', 'Sun', '', WATCH, 2, 1, 1, 1),
                 ('input_select.scene', 'Scene', '', WATCH, 0, 2, 1, 1), ('timer.eggs', 'Eggs', '', WATCH, 1, 2, 1, 1),
                 ('vacuum.cleaning', 'Robot', '', WATCH, 2, 2, 1, 1)]),
    ('a_wide', [('alarm_control_panel.home', 'Alarm', '', {}, 0, 0, 2, 1), ('person.tim', 'Tim', '', {}, 2, 0, 1, 1),
                ('weather.home', 'Weather', '', {}, 0, 1, 3, 1), ('calendar.home', 'Family', '', {}, 0, 2, 2, 1), ('remote.tv', 'TV', '', {'controls': 'toggle'}, 2, 2, 1, 1)]),
    ('a_big', [('weather.home', 'Weather', '', {'display': 'forecast'}, 0, 0, 3, 2), ('sun.sun', 'Sun', '', {'display': 'sunpath'}, 0, 2, 3, 1)]),
    ('a_big2', [('alarm_control_panel.home', 'Alarm', '', {}, 0, 0, 2, 2), ('lock.door', 'Front door', '', {}, 2, 0, 1, 2),
                ('vacuum.cleaning', 'Robot', '', {'controls': 'buttons'}, 0, 2, 2, 1), ('input_number.volume', 'Volume', '', {'controls': 'slider'}, 2, 2, 1, 1)]),
    ('a_guard', [('alarm_control_panel.off', 'Alarm', '', {}, 0, 0, 3, 2), ('lock.open', 'Back door', '', {}, 0, 2, 1, 1),
                 ('lock.door', 'Front door', '', {}, 1, 2, 2, 1)]),
    ('a_guard2', [('alarm_control_panel.home', 'House', '', {}, 0, 0, 1, 3), ('lock.open', 'Garage door', '', {}, 1, 0, 2, 2),
                  ('alarm_control_panel.off', 'Shed', '', {}, 1, 2, 2, 1)]),
    ('w_weather', [('weather.home', 'Weather', '', {}, 0, 0, 1, 1), ('weather.home', 'Outside', '', {}, 1, 0, 2, 1),
                   ('weather.home', 'Forecast', '', {'display': 'forecast'}, 0, 1, 3, 1), ('weather.home', 'Week', '', {'display': 'forecast'}, 0, 2, 2, 1),
                   ('sun.sun', 'Sun', '', {'display': 'sunpath'}, 2, 2, 1, 1)]),
    ('w_weather2', [('weather.home', 'Home', '', {'display': 'forecast'}, 0, 0, 2, 2), ('weather.home', 'Today', '', {}, 2, 0, 1, 2),
                    ('sun.sun', 'Sun', '', {'display': 'sunpath'}, 0, 2, 3, 1)]),
    ('w_weather3', [('weather.home', 'Weather', '', {'display': 'forecast'}, 0, 0, 3, 2), ('weather.home', 'Now', '', {}, 0, 2, 3, 1)]),
    ('w_cal', [('calendar.home', 'Family', '', {}, 0, 0, 1, 1), ('calendar.home', 'Agenda', '', {}, 1, 0, 2, 1),
               ('calendar.home', 'This week', '', {}, 0, 1, 2, 2), ('calendar.home', 'Next', '', {}, 2, 1, 1, 2)]),
    ('w_cal2', [('calendar.home', 'Family', '', {}, 0, 0, 3, 3)]),
    ('a_clocks', [('screen.clock', '', '', {'display': 'flip'}, 0, 0, 2, 2), ('screen.clock', '', '', {'display': 'dial'}, 2, 0, 1, 2),
                  ('screen.clock', '', '', {'display': 'digital'}, 0, 2, 1, 1), ('screen.clock', '', '', {'display': 'dial'}, 1, 2, 2, 1)]),
    ('a_clocks2', [('screen.clock', '', '', {'display': 'analog'}, 0, 0, 2, 2), ('screen.clock', '', '', {'display': 'digital'}, 2, 0, 1, 2),
                   ('screen.clock', '', '', {'display': 'flip'}, 0, 2, 2, 1), ('screen.clock', '', '', {'display': 'analog'}, 2, 2, 1, 1)]),
    ('a_meter', [('sensor.power', 'House power', '', {'display': 'meter'}, 0, 0, 1, 1), ('sensor.humidity', 'Humidity', '', {'display': 'meter'}, 1, 0, 2, 1),
                 ('sensor.temp', 'Living room', '', {'display': 'meter'}, 0, 1, 1, 2), ('sensor.co2', 'Office CO2', '', {'display': 'meter'}, 1, 1, 2, 2)]),
    ('a_builtin', [('screen.settings', '', '', {}, 0, 0, 1, 1), ('screen.page', 'Home', '', {}, 1, 0, 1, 1), ('screen.map', '', '', {}, 2, 0, 1, 1),
                   ('screen.energy', '', '', {}, 0, 1, 2, 2), ('screen.settings', '', '', {}, 2, 1, 1, 2)]),
    ('a_night', [('screen.nightstand', '', '', {}, 0, 0, 3, 3)]),
    ('s_std', [('sensor.temp', 'Living room', '', {}, 0, 0, 1, 1), ('sensor.power', 'House power', '', {}, 1, 0, 2, 1),
               ('sensor.energy', 'Energy today', '', {}, 0, 1, 1, 2), ('sensor.co2', 'Office CO2', '', {}, 1, 1, 2, 2)]),
    ('s_watch', [('sensor.energy', 'Energy', '', WATCH, 0, 0, 1, 1), ('sensor.pressure', 'Pressure', '', WATCH, 1, 0, 1, 1),
                 ('sensor.text', 'Sky', '', WATCH, 2, 0, 1, 1), ('sensor.energy', 'Energy total', '', WATCH, 0, 1, 2, 1),
                 ('sensor.power', 'House power', '', WATCH, 2, 1, 1, 2), ('sensor.pressure', 'Pressure', '', WATCH, 0, 2, 2, 1)]),
    ('s_watch2', [('sensor.energy', 'Energy total', '', WATCH, 0, 0, 3, 1), ('sensor.power', 'House power', '', WATCH, 0, 1, 2, 2),
                  ('sensor.co2', 'CO2', '', WATCH, 2, 1, 1, 2)]),
    ('s_watch3', [('sensor.temp_f', 'Outside temperature', '', WATCH, 0, 0, 3, 2), ('sensor.energy', 'Energy', '', WATCH, 0, 2, 3, 1)]),
    ('s_watch4', [('sensor.power', 'House power', '', WATCH, 0, 0, 3, 3)]),
    ('e_energy', [('screen.energy', '', '', {}, 0, 0, 2, 2), ('screen.energy', '', '', {}, 2, 0, 1, 2), ('screen.energy', '', '', {}, 0, 2, 3, 1)]),
    ('e_energy2', [('screen.energy', '', '', {}, 0, 0, 3, 2), ('screen.energy', '', '', {}, 0, 2, 1, 1), ('screen.energy', '', '', {}, 1, 2, 2, 1)]),
    ('e_energy3', [('screen.energy', '', '', {}, 0, 0, 3, 3)]),
    ('a_screen', [('screen.clock', '', '', {'display': 'digital'}, 0, 0, 2, 1), ('screen.clock', '', '', {'display': 'analog'}, 2, 0, 1, 1),
                  ('screen.clock', '', '', {'display': 'flip'}, 0, 1, 3, 1), ('screen.clock', '', '', {'display': 'dial'}, 0, 2, 1, 1),
                  ('input_select.scene', 'Scene', '', {'controls': 'stepper'}, 1, 2, 2, 1)]),
    ('m_opts', [('media_player.spotify', 'Standard', '', {'controls': 'playback'}, 0, 0, 2, 2),
                ('media_player.spotify', 'No line', '', {'controls': 'playback', 'display': 'cover', 'subtitle': 'none'}, 2, 0, 1, 2),
                ('media_player.spotify', 'No keys', '', {'controls': 'none', 'display': 'cover'}, 0, 2, 2, 1),
                ('media_player.spotify', 'Words', '', {'controls': 'playback', 'subtitle': 'text:Kitchen'}, 2, 2, 1, 1)]),
    ('m_opts2', [('media_player.spotify', 'Both', '', {'controls': 'volume_playback', 'display': 'cover'}, 0, 0, 2, 2),
                 ('media_player.spotify', 'Plain', '', {'controls': 'volume_playback'}, 2, 0, 1, 3),
                 ('media_player.spotify', 'Plain wide', '', {'controls': 'playback'}, 0, 2, 2, 1)]),
    ('m_long', [('media_player.long', 'Living room', '', {'controls': 'playback', 'display': 'cover'}, 0, 0, 2, 2),
                ('media_player.long', 'Kitchen', '', {'controls': 'playback'}, 0, 2, 3, 1), ('media_player.long', 'Bath', '', {'controls': 'playback'}, 2, 0, 1, 2)]),
    ('m_card', [('media_player.spotify', 'Spotify Tim Rousseau', '', {'controls': 'playback', 'display': 'cover'}, 0, 0, 2, 2)]),
    ('m_full_none', [('media_player.spotify', 'Spotify Tim Rousseau', '', {'controls': 'none', 'subtitle': 'none'}, 0, 0, 3, 3)]),
    ('m_full_keys', [('media_player.spotify', 'Spotify Tim Rousseau', '', {'controls': 'playback'}, 0, 0, 3, 3)]),
    ('m_full', [('media_player.spotify', 'Spotify', '', {'controls': 'volume_playback'}, 0, 0, 3, 3)]),
    ('c_media_full', [('media_player.sonos', 'Living room', '', {'controls': 'volume_playback'}, 0, 0, 3, 3)]),
    ('graph_full', [('sensor.graph', 'Outside temperature', '', GRAPH, 0, 0, 3, 3)]),
]


# The cards a tap (or a hold, for a tile a tap switches) opens: page -> [(tile on the page, hold, shot)].
CARDS = {
    'singles': [(0, True, 'k_light'), (2, True, 'k_lock'), (6, True, 'k_blind')],
    'c_climate2': [(0, False, 'k_climate')],
    'c_cover3': [(3, False, 'k_tilt')],
    'c_misc': [(0, False, 'k_select'), (3, True, 'k_timer')],
    'c_misc2': [(0, True, 'k_vacuum'), (2, True, 'k_fan')],
    'a_big2': [(0, False, 'k_alarm')],
    'graphs': [(0, False, 'k_graph')],
    'w_weather': [(0, False, 'k_weather')],
    'w_cal': [(1, False, 'k_calendar')],
    'c_misc3': [(2, True, 'k_humidifier')],
}


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
        states, pages, extras_of = {}, [], {}
        for page, (title, cells) in enumerate(PAGES):
            tiles = []
            for n, (state, name, icon, options, c, r, w, h) in enumerate(cells):
                domain = state.split('.')[0]
                # The screen's own cards (the clock) keep their own entity and have no state.
                entity = state if domain == 'screen' else f'{domain}.{state.split(".")[1]}_p{page}_{n}'
                if domain != 'screen':
                    states[entity] = STATES[state]
                if state in EXTRAS:
                    extras_of[entity] = EXTRAS[state]
                # screen.page: a tile that goes to the home page (navigation); screen.<card>: the screen's own card.
                content = ({'kind': 'navigation', 'target': {'kind': 'home'}} if entity == 'screen.page' else
                           {'kind': 'builtin', 'name': entity[7:]} if domain == 'screen' else {'kind': 'entity', 'entityId': entity})
                tiles.append({'id': f'a{page:03x}{n:012x}', 'content': content,
                              'placement': {'row': r, 'column': c, 'columns': w, 'rows': h},
                              'appearance': {'label': name, 'presentation': page_layout.footprint_size(w, h, grid),
                                             **({'icon': icon} if icon in CHOSEN else {}),
                                             **{k: v for k, v in options.items() if k in APPEARANCE}},
                              'interaction': {k: v for k, v in options.items() if k not in APPEARANCE}})
            pages.append({'id': f'b{page:015x}', 'navigation': {'excludeFromPagination': False},
                          'topbar': {'leading': [], 'title': {'source': 'text', 'text': title}, 'trailing': []}, 'tiles': tiles})
        record = {'format': 'pages-v2', 'sourceGrid': {'columns': 3, 'rows': 3},
                  'layout': {'title': 'Soft', 'homePageId': pages[0]['id'], 'pages': pages}}
        compiled = send_layout.compile_tiles(record['layout'], grid)
        import core
        values = [send_layout.state_message(i, tile, states, core.media_extras(states[tile['entity']]['attributes']) if tile['entity'].startswith('media_player.')
                                            else extras_of.get(tile['entity']))
                  for i, tile in enumerate(compiled)]
        for message, tile in zip(values, compiled):
            if tile['entity'].startswith('sensor.graph'):
                message['history'] = HISTORY
        bars = [[{'k': 'clock'}] for _ in record['layout']['pages']]
        await self.sender.synchronize(self.inbox.object_id, record, region, values, bars)
        # Covers, as the app paints them for a tile's place (tile_picture.h): every question answered as it comes.
        import media_art
        import tile_art
        calls = []
        self.client.subscribe_service_calls(calls.append)
        async def covers():
            served, seen = 0, 0
            while True:
                for call in calls[seen:]:
                    seen += 1
                    data = dict(call.data)
                    if call.service == 'esphome.screen_camera' and data.get('size') and 'tiles' not in data:
                        # The player's card asks for its cover at a size of its own (camera_feed.encode_cover).
                        import camera_feed
                        body = camera_feed.encode_cover(media_art.png(1, 480), int(data['size']), int(data['bg'], 16))
                        url = self.pictures.url(f'{self.item.key}-cover-{served}.bmp', body)
                        served += 1
                        await self.send({'v': 1, 'op': 'camera', 't': 'cover', 'e': data['entity'], 'u': url, 'view': int(data['view'])})
                        continue
                    if call.service != 'esphome.screen_camera' or 'tiles' not in data:
                        continue
                    entities = data['tiles'].split(',')
                    atlas = tile_art.parse(data.get("atlas"), (4096, 4096), len(entities)) if data.get("atlas") else None
                    if atlas is None:
                        continue
                    seed = 4 if 'paused' in entities[0] else 1
                    body = tile_art.encode([media_art.png(seed)] * len(entities), [int(g, 16) for g in data['bg'].split(',')], atlas, compact=True)
                    url = self.pictures.url(f'{self.item.key}-tile-{served}.bmp', body)
                    served += 1
                    await self.send({'v': 1, 'op': 'camera', 't': 'live', 'e': data['tiles'], 'u': url, 'view': int(data['view'])})
                await asyncio.sleep(0.1)
        asyncio.get_running_loop().create_task(covers())
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
            await asyncio.sleep(3.0 if name.startswith('m_') else 0.5)  # a player's cover comes after the page
            await self.render(name, timeout=6)
            for n, hold, shot in CARDS.get(name, ()):
                index = next(i for i, tile in enumerate(compiled) if tile['entity'].endswith(f'_p{page}_{n}'))
                start = len(self.lines)
                await self.call('render_tile_center', index=index)
                line = await self.until(lambda l: f'tile_center index={index}' in l, 5, f'tile {index}', start)
                x, y = (int(part.split('=')[1]) for part in line.split()[-2:])
                await self.call('render_finger', x=x, y=y, down=True)
                await asyncio.sleep(1.2 if hold else 0.15)
                await self.call('render_finger', x=x, y=y, down=False)
                await asyncio.sleep(2.0)
                await self.render(shot, timeout=6)
                await self.call('render_close_cards')
                await asyncio.sleep(0.6)
            if name == 'm_card':
                # The player's card, opened as a finger does: a tap on the tile's words.
                index = next(i for i, tile in enumerate(compiled) if tile['entity'].endswith(f'_p{page}_0'))
                start = len(self.lines)
                await self.call('render_tile_center', index=index)
                line = await self.until(lambda l: f'tile_center index={index}' in l, 5, 'the player tile', start)
                x, y = (int(part.split('=')[1]) for part in line.split()[-2:])
                await self.tap(x, y)
                await asyncio.sleep(3.0)
                await self.render('m_card_open', timeout=6)
                # A tap on the cover: the big cover, and a second tap back.
                await self.tap(240, 150)
                await asyncio.sleep(3.0)
                await self.render('m_card_big', timeout=6)
                await self.tap(240, 150)
                await asyncio.sleep(1.0)
                await self.call('render_close_cards')
                await asyncio.sleep(0.5)
        for step in [x for x in os.environ.get('SOFT_EXTRAS', '').split(',') if x]:
            await getattr(self, f'extra_{step}')()
        return len(PAGES), len(PAGES)

    async def extra_saver(self):
        """The screensaver in Soft: the clock, the clock with its row, and a player's cover (run.py's saver steps)."""
        import camera_feed
        import media_art
        import media_library
        calls = []
        self.client.subscribe_service_calls(calls.append)
        ground = media_library.ground_colours(media_art.png(0)) or ''
        row = [{'k': 'text', 't': '21°'}, {'k': 'text', 'i': 'F0335', 't': ''}, {'k': 'text', 'i': 'F140B', 't': '412 W'}]
        steps = [('clock', {'k': 'clock'}), ('clock-row', {'k': 'clock', 'w': '21°', 'wi': row}),
                 ('media', {'k': 'media', 'e': 'media_player.living_room', 'n': 'Living room', 't': 'Evening Drive',
                            'x': {'artist': 'Nova Coast', 'album': 'Low Sun', 'pic': 'c0ffee1234', 'g': ground}})]
        await self.call('render_standby', enter=1)
        for name, message in steps:
            start = len(calls)
            await self.send({'v': 1, 'op': 'saver', **message})
            end = time.monotonic() + 6
            while name == 'media' and time.monotonic() < end:
                ask = next((c for c in calls[start:] if c.service == 'esphome.screen_camera' and dict(c.data).get('saver')), None)
                if ask:
                    data = dict(ask.data)
                    box = camera_feed.capped((480, 480))
                    body = camera_feed.encode_saver(media_art.png(0, 640), box, 'media', int(ground.split(',')[0], 16) if ground else 0)
                    url = self.pictures.url(f'{self.item.key}-saver.bmp', body)
                    await self.send({'v': 1, 'op': 'camera', 't': 'saver', 'e': data['entity'], 'u': url, 'view': int(data['view'])})
                    break
                await asyncio.sleep(0.1)
            await asyncio.sleep(2.0)
            await self.render(f'x_saver_{name}', timeout=6)
        await self.call('render_standby', enter=0)
        await asyncio.sleep(1.0)

    async def extra_settings(self):
        """The screen's own settings page in Soft, and its first group."""
        await self.call('open_settings', page=0)
        await asyncio.sleep(1.2)
        await self.render('x_settings', timeout=6)
        await self.call('open_settings', page=1)
        await asyncio.sleep(1.2)
        await self.render('x_settings_1', timeout=6)



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

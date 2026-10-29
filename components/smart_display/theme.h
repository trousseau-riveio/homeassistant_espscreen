#pragma once
// Every colour the screens choose themselves, in one table (firmware 0.2.54+).
//
// A role is what a colour is for: the page, a card, the text on it, a key under a finger. Each role has its light value,
// the look the screens have always had, and its dark value, for a screen beside a bed: a black page, graphite cards,
// soft white text, and the colours of Home Assistant's states where they matter. Nothing else in the firmware or in the
// board profiles writes a colour of its own; the profiles name a paint (a shared LVGL style filled from this table) and
// the firmware asks for a role. docs/THEME.md is the recipe for a new colour.
//
// Home Assistant's state colours (the amber of a light that is on, the orange of heating) are not roles: they are data
// and pass through state() on their way to the glass. The values below are free of LVGL, so tests/test_theme.cpp checks
// the table and the contrast of both looks on a PC; the LVGL half is behind THEME_TEST.
#include <cstddef>
#include <cstdint>
#include <string>

namespace theme {

enum Role : uint8_t {
  // ---- surfaces
  PAGE,                 // the page behind the tiles, every card and the settings
  PAGE_SOFT,            // the page of the light's colour controls, a shade lighter
  PAGE_PRESSED,         // the Previous and Next bars under a finger
  CARD,                 // tiles, cards, rows, the alert
  CARD_PRESSED,         // a settings row under a finger
  LINE,                 // the hairline around a card
  RAISED,               // a card above everything else: the alert
  RAISED_LINE,
  KEY,                  // round keys (back, close), a white key under a finger
  KEY_PRESSED,          // a round key under a finger
  TRACK,                // segment tracks and grey keys inside a card, an unavailable tile's circle, empty history
  GRID,                 // the lines across a history graph
  TICK,                 // the small marks under a history graph
  BUTTON,               // the plain keys of a simple card (select options, media, timer)
  SETTING_KEY,          // -/+ keys and choice chips on the settings page
  SETTING_KEY_PRESSED,
  TOGGLE_OFF,           // a switch on the settings page while off
  SWITCH_OFF,           // the switch on a history card while off
  PANEL_TOGGLE_OFF,     // the toggle on a wide tile while off
  PANEL_TOGGLE_OFF_PRESSED,
  KNOB,                 // handles and knobs
  STEPPER_KEY,          // the - and + keys inside a thermostat's or a number's grey stepper (firmware 0.3.3)
  STEPPER_KEY_PRESSED,
  SLIDER_KNOB,          // the round knob of a card's number or volume slider
  SCRIM,                // behind an alert
  VEIL,                 // the sheet over a busy tile, the haze over the value card
  SPINNER_TRACK,        // the ring of the busy spinner
  // ---- text
  INK,                  // names, values, words on keys
  INK_SOFT,             // the alert's second line
  MUTED,                // secondary words: states, times beside a value
  SUBTLE,               // quiet words: axis labels, notes, "Target"
  SLATE,                // the top bar, units, a value on a coloured card, mode key icons
  BEDSIDE,              // the bedside clock's digits: ink by day, a soft grey beside a bed at night (firmware 0.8.0)
  ROW_ICON,             // icons at the start of a settings row
  CHEVRON,              // the arrow at the end of a settings row
  OFF,                  // something that is off: its icon, its circle and the fill of its slider
  ON_ACCENT,            // words and icons on a blue key and on the alert's key
  // ---- keys that stand out
  ACCENT,               // the screen's own blue: a chosen segment, the main key, a switch that is on
  ACCENT_PRESSED,
  ACCENT_BRIGHT,        // the value card's fill and a chosen climate chip
  ACCENT_TINT,          // pale blue: a choice under a finger, the value card's track, climate power on
  ACCENT_ICON,          // the climate power icon while on
  ICON_OFF,             // the climate power icon while off
  BUTTON_DARK,          // the alert's key
  BUTTON_DARK_PRESSED,
  // ---- drawings
  ROBOT_BODY,           // the vacuum seen from above
  ROBOT_RIM,
  ROBOT_TOP,
  ROBOT_LENS,
  BATTERY,              // a battery's outline and cap
  SUN_PATH,             // the arc the sun travels
  MOON,                 // the sun below the horizon
  AMBER_TRACK,          // the track of a light's brightness slider
  HISTORY_OFF,          // an off or away stretch on a timeline (the add-on's CFCFCF)
  HISTORY_EMPTY,        // a stretch without data on a timeline (the add-on's E6E6E6)
  // ---- a camera image: dark in both looks, like a photo viewer (firmware 0.2.57+)
  CAMERA_PAGE,          // behind the image full screen and in the alert's image frame
  CAMERA_INK,           // the camera's name over the image
  CAMERA_NOTE,          // "No image from this camera"
  CAMERA_TRACK,         // the ring of the spinner while the first image loads (firmware 0.2.73+)
  // ---- the media card: a dark ground from its cover's colour in both looks (firmware 0.24.0+); this is its ground
  // while the app has read no colour, and a cover's own takes its place once it has
  MEDIA_GROUND,
  // ---- the CYD's touch calibration: dark in both looks, a yellow crosshair
  CALIBRATION_PAGE,
  CALIBRATION_INK,
  CALIBRATION_MARK,
  // ---- the hotspot's QR code on the Wi-Fi problem screen (firmware 0.38.0): black on white in both looks, as a phone's
  // camera reads a QR code most surely, with a white quiet zone around it
  QR_DARK,
  QR_LIGHT,
  // ---- the Tessera mark's four colours (docs/images/tessera-mark.svg), the same in both looks: the mosaic a screen shows
  // while it waits for its first tiles (firmware 0.45.0)
  MARK_AMBER,
  MARK_BLUE,
  MARK_PURPLE,
  MARK_GREEN,
  // ---- the energy card (firmware 0.47.0): Home Assistant's energy colours (--energy-*-color), the same in both looks
  // but the power going back to the grid, which Home Assistant lightens in its dark theme; and power-flow-card-plus's
  // four device colours, in the order of their places (above the house, below it, top right, bottom right)
  ENERGY_SOLAR,
  ENERGY_GRID,
  ENERGY_RETURN,
  ENERGY_BATTERY_OUT,
  ENERGY_BATTERY_IN,
  ENERGY_DEVICE_1,
  ENERGY_DEVICE_2,
  ENERGY_DEVICE_3,
  ENERGY_DEVICE_4,
  ENERGY_REST,           // Other and Untracked consumption: Home Assistant's --state-unavailable-color
  // ---- the weather card's two-tone icons (its week, weather_chart.h): a cloud in front, a second cloud behind it, as Home
  // Assistant's own weather pictures draw them; a sun, drops and flakes keep their condition colours
  WEATHER_CLOUD,
  WEATHER_CLOUD_BACK,
  // ---- the screensaver's clock faces (module clock_faces, docs/SCREENSAVER.md): white hands on either face, and the
  // Bauhaus face's five colours, a little deeper in its dark look. A face reads the light or the dark value by its own
  // look, which need not be the screen's.
  CLOCK_HAND,
  BAUHAUS_RED,
  BAUHAUS_YELLOW,
  BAUHAUS_BLUE,
  BAUHAUS_BLACK,
  BAUHAUS_CREAM,
  ROLE_COUNT
};

struct Pair { uint32_t light, dark; };

// light: the design as it always was. dark: RGB565-friendly values (a grey whose red, green and blue land on the same
// step of the panel), so dark greys stay neutral on both boards instead of turning green.
inline constexpr Pair ROLES[ROLE_COUNT] = {
  /* PAGE */                     {0xE7E7E7, 0x000000},
  /* PAGE_SOFT */                {0xF8F8F8, 0x101010},
  /* PAGE_PRESSED */             {0xE3E3E3, 0x1A1A1A},
  /* CARD */                     {0xFFFFFF, 0x1A1A1A},
  /* CARD_PRESSED */             {0xF1F1F1, 0x2A2A2A},
  /* LINE */                     {0xDDDDDD, 0x292929},
  /* RAISED */                   {0xFFFFFF, 0x282828},
  /* RAISED_LINE */              {0xDDDDDD, 0x393939},
  /* KEY */                      {0xEEEEEE, 0x2A2A2A},
  /* KEY_PRESSED */              {0xDDDDDD, 0x3A3A3A},
  /* TRACK */                    {0xF1F1F1, 0x2A2A2A},
  /* GRID */                     {0xEEEEEE, 0x292929},
  /* TICK */                     {0xCCCCCC, 0x4A4A4A},
  /* BUTTON */                   {0xD9E6F0, 0x222B32},
  /* SETTING_KEY */              {0xE9EFF4, 0x2A2F34},
  /* SETTING_KEY_PRESSED */      {0xC3D5E3, 0x3A4652},
  /* TOGGLE_OFF */               {0xC4C4C4, 0x484848},
  /* SWITCH_OFF */               {0xB0B0B0, 0x484848},
  /* PANEL_TOGGLE_OFF */         {0xD7DADF, 0x393C41},
  /* PANEL_TOGGLE_OFF_PRESSED */ {0xC5C9CF, 0x494D52},
  /* KNOB */                     {0xFFFFFF, 0xE8E8E8},
  /* STEPPER_KEY */              {0xFFFFFF, 0x3A3A3A},
  /* STEPPER_KEY_PRESSED */      {0xE4E4E4, 0x4A4A4A},
  /* SLIDER_KNOB */              {0x111111, 0xE8E8E8},
  /* SCRIM */                    {0x101820, 0x000000},
  /* VEIL */                     {0xFFFFFF, 0x1A1A1A},
  /* SPINNER_TRACK */            {0xD9DDE2, 0x393D42},
  /* INK */                      {0x1B1B1B, 0xDADADA},
  /* INK_SOFT */                 {0x3A3A3C, 0xBABABA},
  /* MUTED */                    {0x616161, 0x999999},
  /* SUBTLE */                   {0x6B6B6B, 0x8A8A8A},
  /* SLATE */                    {0x46525E, 0x9AA6B2},
  /* BEDSIDE */                  {0x303030, 0x888888},
  /* ROW_ICON */                 {0x4A4A4A, 0xB8B8B8},
  /* CHEVRON */                  {0xB0B0B0, 0x5A5A5A},
  /* OFF */                      {0x9E9E9E, 0x787878},
  /* ON_ACCENT */                {0xFFFFFF, 0xFFFFFF},
  /* ACCENT */                   {0x009FE3, 0x0A93D2},
  /* ACCENT_PRESSED */           {0x0075B0, 0x0A6FA3},
  /* ACCENT_BRIGHT */            {0x00A6ED, 0x0A9ADB},
  /* ACCENT_TINT */              {0xD5EEFC, 0x12384A},
  /* ACCENT_ICON */              {0x0B6E99, 0x6CCBF5},
  /* ICON_OFF */                 {0x8A8A8A, 0x7A7A7A},
  /* BUTTON_DARK */              {0x1B1B1B, 0x484848},
  /* BUTTON_DARK_PRESSED */      {0x3A3A3C, 0x585858},
  /* ROBOT_BODY */               {0xFFFFFF, 0x383D42},
  /* ROBOT_RIM */                {0xCEDDE6, 0x5A6972},
  /* ROBOT_TOP */                {0xE3EBEF, 0x4A5358},
  /* ROBOT_LENS */               {0xA8BCC8, 0x7E93A1},
  /* BATTERY */                  {0x7D858D, 0x8C959D},
  /* SUN_PATH */                 {0xCFD8DC, 0x3E494E},
  /* MOON */                     {0xB0BEC5, 0x90A4AE},
  /* AMBER_TRACK */              {0xFDEFC3, 0x483B16},
  /* HISTORY_OFF */              {0xCFCFCF, 0x4A4A4A},
  /* HISTORY_EMPTY */            {0xE6E6E6, 0x2A2A2A},
  /* CAMERA_PAGE */              {0x000000, 0x000000},
  /* CAMERA_INK */               {0xF2F2F2, 0xDADADA},
  /* CAMERA_NOTE */              {0x9E9E9E, 0x8A8A8A},
  /* CAMERA_TRACK */             {0x393D42, 0x393D42},
  /* MEDIA_GROUND */             {0x000000, 0x000000},
  /* CALIBRATION_PAGE */         {0x101820, 0x101820},
  /* CALIBRATION_INK */          {0xFFFFFF, 0xFFFFFF},
  /* CALIBRATION_MARK */         {0xFFD34D, 0xFFD34D},
  /* QR_DARK */                  {0x000000, 0x000000},
  /* QR_LIGHT */                 {0xFFFFFF, 0xFFFFFF},
  /* MARK_AMBER */               {0xFFC107, 0xFFC107},
  /* MARK_BLUE */                {0x009FE3, 0x009FE3},
  /* MARK_PURPLE */              {0x926BC7, 0x926BC7},
  /* MARK_GREEN */               {0x4CAF50, 0x4CAF50},
  /* ENERGY_SOLAR */             {0xFF9800, 0xFF9800},
  /* ENERGY_GRID */              {0x488FC2, 0x488FC2},
  /* ENERGY_RETURN */            {0x8353D1, 0xA280DB},
  /* ENERGY_BATTERY_OUT */       {0x4DB6AC, 0x4DB6AC},
  /* ENERGY_BATTERY_IN */        {0xF06292, 0xF06292},
  /* ENERGY_DEVICE_1 */          {0xD0CC5B, 0xD0CC5B},
  /* ENERGY_DEVICE_2 */          {0x964CB5, 0x964CB5},
  /* ENERGY_DEVICE_3 */          {0xB54C9D, 0xB54C9D},
  /* ENERGY_DEVICE_4 */          {0x5BD0CC, 0x5BD0CC},
  /* ENERGY_REST */              {0xBDBDBD, 0x707070},  // dark: #6F6F6F on the panel's grey step
  /* WEATHER_CLOUD */            {0xB0BEC5, 0xCFD8DC},
  /* WEATHER_CLOUD_BACK */       {0x90A4AE, 0x78909C},
  /* CLOCK_HAND */               {0xFFFFFF, 0xFFFFFF},
  /* BAUHAUS_RED */              {0xD9442B, 0xB23A26},
  /* BAUHAUS_YELLOW */           {0xF2B33D, 0xC9932F},
  /* BAUHAUS_BLUE */             {0x2B4E9B, 0x284783},
  /* BAUHAUS_BLACK */            {0x1D1D1B, 0x0B0B0B},
  /* BAUHAUS_CREAM */            {0xF2EBDD, 0x1A1A19},
};

// The look on screen. The board sets it through set_dark() (below) from the Dark mode setting; the table and the
// helpers only read it.
inline bool dark = false;
// The Glow look (local fork, settings_screen::theme): a card that is on takes a soft wash of its state colour, its
// corners are sharper. The board sets it through runtime_tiles::theme_changed().
inline bool glow = false;
// Glow's card corner, in the look's pixels (ui::px), against the look's TILE_RADIUS.
inline constexpr int GLOW_RADIUS = 8;
// The Vision look (local fork, settings_screen::theme 2), after visionOS: glass cards on a dark slate page, a card that
// is on turns to glass of its state colour (amber for a lamp), the icon at the top left without a circle and the name
// under it at the left in a medium face. It is always dark, so every word on every card is light and reads on both
// glasses; the Dark mode setting stays what it is and counts again in the other looks.
inline bool vision = false;
inline constexpr int VISION_RADIUS = 16;
// Glow and Vision draw no circle behind a tile's icon.
inline bool bare() { return glow || vision; }
// The Soft look (local fork, settings_screen::theme 3): warm paper cards on a warm page, the icon in a grey circle at
// the top left, the state at the top right (green while on), a large name and, for anything with an on and an off,
// a pill switch with an orange knob at the bottom (runtime_tiles). It is light or dark as its colours are (settings_screen::soft_bg); Dark mode counts again in the
// other looks.
inline bool soft = false;
inline constexpr int SOFT_RADIUS = 18;
// Soft's colours (settings_screen::soft_bg): the page, a card and its hairline, the icon's circle and the switch's track,
// the words, the state word while on, and the accent of the switch's knob, a slider and an icon that is on.
// A dark palette draws the rest of the screen dark too (drawn_dark): its top bar, its keys and its cards' pop-ups.
struct SoftPalette { const char *name; uint32_t page, card, line, circle, ink, muted, on, knob; bool dark = false; };
inline constexpr SoftPalette SOFT_PALETTES[] = {
  // A page a few shades deeper than its cards, so each card stands out; dark words, and an accent strong on white.
  {"Warm", 0xDED8CC, 0xFCFBF8, 0xD6D0C4, 0xECE8E0, 0x141414, 0x5E5B56, 0x4E8F2E, 0xF29A33},
  {"Sage", 0xC9D6C3, 0xFBFCFA, 0xC4CFBF, 0xE3EADF, 0x141A13, 0x545C51, 0x3A7A33, 0x5E9A4C},
  {"Ocean", 0xC5D5E5, 0xFBFCFE, 0xBFCCDA, 0xE2EAF2, 0x121822, 0x4F5A66, 0x23649F, 0x3F8BD0},
  {"Rose", 0xE6CFD0, 0xFDFAFA, 0xDCC6C7, 0xF1E4E4, 0x1C1314, 0x66585A, 0x9E3F4C, 0xE2697A},
  {"Lavender", 0xD3CEE6, 0xFCFBFE, 0xCBC6DE, 0xE8E5F2, 0x16141F, 0x5A5668, 0x5643A8, 0x7E6AD8},
  {"Sand", 0xE0D2B8, 0xFCF9F3, 0xD6C9AF, 0xEEE5D4, 0x1A160F, 0x625A4C, 0x8A5D16, 0xD18E2B},
  {"Stone", 0xCFCFCF, 0xFFFFFF, 0xC8C8C8, 0xEBEBEB, 0x111111, 0x555555, 0x1F1F1F, 0x262626},
  // The same, dark: a near-black page, cards a step lighter, light words; the accents a touch brighter to carry on dark.
  {"Warm Dark", 0x1A1917, 0x282623, 0x36332E, 0x35322D, 0xF2EFEA, 0xA39E96, 0x8CC66A, 0xF29A33, true},
  {"Sage Dark", 0x151A15, 0x222922, 0x2F382E, 0x2D372C, 0xEEF2EC, 0x9CA699, 0x86C474, 0x74B260, true},
  {"Ocean Dark", 0x12171E, 0x1D2531, 0x293444, 0x273345, 0xEDF2F7, 0x98A4B2, 0x6FB0EA, 0x4D9BE0, true},
  {"Rose Dark", 0x1C1516, 0x2B2123, 0x3B2E31, 0x392B2E, 0xF6EEEF, 0xAE9C9F, 0xEE8E9B, 0xE2697A, true},
  {"Lavender Dark", 0x17151F, 0x242130, 0x322E42, 0x302C40, 0xF1EFF8, 0xA49FB6, 0xA898F0, 0x8C78E6, true},
  {"Sand Dark", 0x1B1813, 0x29251D, 0x383227, 0x363025, 0xF4EFE6, 0xA89F8E, 0xE2B15E, 0xD18E2B, true},
  {"Graphite", 0x141414, 0x222222, 0x303030, 0x2E2E2E, 0xF2F2F2, 0x9E9E9E, 0xE6E6E6, 0xD6D6D6, true},
};
inline constexpr int SOFT_PALETTE_COUNT = sizeof(SOFT_PALETTES) / sizeof(SOFT_PALETTES[0]);
inline constexpr uint32_t SOFT_KNOB_OFF = 0xFFFFFF;
inline int soft_palette = 0;
inline const SoftPalette &soft_colors() { return SOFT_PALETTES[soft_palette >= 0 && soft_palette < SOFT_PALETTE_COUNT ? soft_palette : 0]; }
// Vision's colours. The page is a gradient (vision_gradient, settings_screen::vision_bg); a card is white glass over
// it (a share of white, and a hairline of a little more); a card that is on is glass of its state colour, a little
// more opaque, with a hairline of that colour lighter. The words are light on both.
inline constexpr uint32_t VISION_PAGE = 0x22272D, VISION_PAGE_TOP = 0x343B44;
inline constexpr uint32_t VISION_GLASS = 0xFFFFFF;
// The page's gradients (settings_screen::vision_bg), lighter at the top, each dark enough for light words and glass;
// the last one is the owner's own two colours (settings_screen::vision_top, vision_bottom).
struct VisionGradient { const char *name; uint32_t top, bottom; };
inline constexpr VisionGradient VISION_GRADIENTS[] = {
  {"Slate", 0x343B44, 0x22272D},
  {"Ocean", 0x1E4467, 0x0E1A2B},
  {"Aurora", 0x1D5450, 0x18213A},
  {"Dusk", 0x553366, 0x1B1C35},
  {"Ember", 0x66392A, 0x201A1D},
  {"Forest", 0x2E5238, 0x141D17},
  {"Graphite", 0x3C3C3F, 0x1A1A1C},
  {"Custom", 0x343B44, 0x22272D},
};
inline constexpr int VISION_GRADIENT_COUNT = sizeof(VISION_GRADIENTS) / sizeof(VISION_GRADIENTS[0]);
inline constexpr int VISION_CUSTOM = VISION_GRADIENT_COUNT - 1;
// The owner's own colour, or the first gradient's where none was set (settings_screen::vision_top/_bottom are -1).
inline uint32_t vision_own(int32_t color, bool top) { return color >= 0 ? (uint32_t) color : top ? VISION_GRADIENTS[0].top : VISION_GRADIENTS[0].bottom; }
inline constexpr uint8_t VISION_GLASS_OPA = 34, VISION_GLASS_LINE_OPA = 46, VISION_LIT_OPA = 112, VISION_LIT_LINE_OPA = 150;
inline constexpr uint32_t VISION_INK = 0xF2F3F5, VISION_MUTED = 0xA9AFB7, VISION_ICON_OFF = 0xD4D8DD, VISION_LIT_MUTED = 0xE6E3DC;
// The Dark mode setting as it was asked for; `dark` is what is drawn, and Vision is always dark (set_vision).
inline bool dark_wanted = false;
inline uint32_t hex(Role role) {
  if (role == PAGE && vision) return VISION_PAGE;
  if (role == PAGE && soft) return soft_colors().page;
  return dark ? ROLES[role].dark : ROLES[role].light;
}

// ---- mixing, exactly as LVGL 9 mixes two colours (lv_color_mix), so a value here is the value on the glass
inline uint32_t mix(uint32_t first, uint32_t second, uint8_t amount) {
  uint32_t out = 0;
  for (int shift = 16; shift >= 0; shift -= 8) {
    const uint32_t a = first >> shift & 0xFF, b = second >> shift & 0xFF;
    out |= (((a * amount + b * (255 - amount)) * 0x8081U) >> 23) << shift;
  }
  return out;
}

// ---- named card colours: what a tile's background or an alert's colour can be
// The add-on sends a name; a tile keeps the light value as the colour's identity (tile_palette.h), and surface() turns it
// into what this look draws. Light: the pastels chosen for dark text. Dark: the same hue deep enough for light text.
struct Swatch { const char *name; uint32_t light, dark; };
inline constexpr Swatch SWATCHES[] = {
  {"red", 0xFADADD, 0x4A2422},
  {"orange", 0xFFE1C6, 0x472F17},
  {"yellow", 0xFFF0C2, 0x3E3516},
  {"green", 0xD9EEDC, 0x243A26},
  {"mint", 0xD5F0EA, 0x163631},
  {"blue", 0xD9EAFB, 0x1C3448},
  {"purple", 0xE9DDF5, 0x342C40},
  {"pink", 0xF7DDEC, 0x46202E},
  {"gray", 0xE5E7EB, 0x2A2D32},
};
inline constexpr size_t SWATCH_COUNT = sizeof(SWATCHES) / sizeof(SWATCHES[0]);
inline uint32_t swatch(const std::string &name) {
  for (const auto &s : SWATCHES) if (name == s.name) return s.light;
  return 0;
}
// The same names on an alert's button (firmware 0.3.3+): a full key colour instead of a pastel, so a green Accept and a
// red Decline stand out on any card. Like Home Assistant's state colours they are the same in both looks; the words on
// them are white, or ink on the light ones (yellow), whichever reads.
struct KeySwatch { const char *name; uint32_t key, text; };
inline constexpr KeySwatch KEY_SWATCHES[] = {
  {"red", 0xD93A30, 0xFFFFFF},
  {"orange", 0xEF7D14, 0xFFFFFF},
  {"yellow", 0xF6C433, 0x1B1B1B},
  {"green", 0x3C9A4A, 0xFFFFFF},
  {"mint", 0x13897B, 0xFFFFFF},
  {"blue", 0x1F7FD6, 0xFFFFFF},
  {"purple", 0x7B55BE, 0xFFFFFF},
  {"pink", 0xD3437E, 0xFFFFFF},
  {"gray", 0x6B7078, 0xFFFFFF},
};
// The key colour of a name, 0 for none or an unknown one (the button keeps its own paint).
inline uint32_t key_swatch(const std::string &name) {
  for (const auto &s : KEY_SWATCHES) if (name == s.name) return s.key;
  return 0;
}
inline uint32_t key_text(uint32_t key) {
  for (const auto &s : KEY_SWATCHES) if (key == s.key) return s.text;
  return 0xFFFFFF;
}
// The key under a finger: a little darker, in both looks.
inline uint32_t key_pressed(uint32_t key) { return mix(key, 0x000000, 210); }
// A card's own colour (a swatch's light value, or 0 for none) as this look draws it.
inline uint32_t surface(uint32_t own) {
  if (!own) return hex(CARD);
  if (!dark) return own;
  for (const auto &s : SWATCHES) if (s.light == own) return s.dark;
  return hex(CARD);
}
// The hairline around a card: the card's own colour a little darker in light, a little lighter in dark.
inline uint32_t outline(uint32_t own) {
  if (!own) return hex(LINE);
  return dark ? mix(surface(own), 0xFFFFFF, 232) : mix(own, 0x000000, 220);
}
// The same hairline around a Glow card's wash (local fork), which is already the colour this look draws.
inline uint32_t outline_of(uint32_t drawn) { return dark ? mix(drawn, 0xFFFFFF, 232) : mix(drawn, 0x000000, 220); }

// ---- Home Assistant's state colours
// What a state looks like in Home Assistant's frontend (its --state-*-color and --*-color variables), the same in both
// looks: an amber light is amber at night too. They are data, drawn through state(), foreground(), tint() and icon().
namespace ha {
constexpr uint32_t AMBER = 0xFFC107;         // on: a light, switch, running script or timer, streaming camera; the sun; lux
constexpr uint32_t ORANGE = 0xFF9800;        // a paused robot, drying, a battery half full, the sun path by day
constexpr uint32_t DEEP_ORANGE = 0xFF6F22;   // temperature, heating
constexpr uint32_t RED = 0xF44336;           // an alarm going off, a robot in trouble, an empty battery
constexpr uint32_t PURPLE = 0x926BC7;        // covers, scenes, energy
constexpr uint32_t DEEP_PURPLE = 0x6E41AB;   // a clear night
constexpr uint32_t INDIGO = 0x3F51B5;        // selects, the sun below the horizon, pouring rain
constexpr uint32_t BLUE = 0x2196F3;          // cooling, rain, a robot on its way back, someone in a zone, everything else
constexpr uint32_t LIGHT_BLUE = 0x03A9F4;    // media players, sleet
constexpr uint32_t CYAN = 0x00BCD4;          // fans, hail
constexpr uint32_t TEAL = 0x009688;          // numbers, percentages, a robot at work
constexpr uint32_t GREEN = 0x4CAF50;         // someone at home, auto mode, wind, a full battery
constexpr uint32_t GREY = 0x9E9E9E;          // anything off or away, fog
// The rest of the weather, as Home Assistant colours a condition (--state-weather-*-color): clouds, a partly
// cloudy sky, lightning, lightning with rain, and snow, which Home Assistant writes as a value of its own.
constexpr uint32_t LIGHT_GREY = 0xBDBDBD, BLUE_GREY = 0x607D8B, YELLOW = 0xFFEB3B, LIME = 0xCDDC39, ICE = 0xC0E0FF;
constexpr uint32_t SKY = 0x00A6ED;           // a robot that is docked or idle
constexpr uint32_t SWITCH_ON = 0xFFB900;     // the switch on a history card while on
constexpr uint32_t PURPLE_PRESSED = 0x7552A8;  // a chosen cover key under a finger
// The weather card's conditions and the sun card.
constexpr uint32_t SUNNY = 0xFFB300, RAIN = 0x1E88E5, SNOW = 0x4FC3F7, LIGHTNING = 0xFFA000;
constexpr uint32_t PARTLY_CLOUDY = 0x7E9BB5, CLOUDY = 0x78909C, NIGHT_SKY = 0x5C6BC0;
// Warnings and good news drawn on a card: a low battery, the analog clock's second hand; charging.
constexpr uint32_t ALARM = 0xE53935, CHARGING = 0x43A047;
}  // namespace ha

// ---- Home Assistant's colours on their way to the glass
// Grey is what Home Assistant draws for something off; the add-on's timelines add a lighter grey for off and one for no
// data. In dark those greys come from the table; every other state colour stays itself.
// A temperature as a colour, from Home Assistant's own hues: indigo in a frost, blue and cyan in the cold, green in
// the mild, amber and orange in the warm, red in a heatwave. The weather tile draws a day's range from its low to its
// high in these (firmware 0.3.3), so a cold day reads as a cold day before the digits do. Degrees Celsius.
inline uint32_t temperature(float celsius) {
  struct Stop { float at; uint32_t color; };
  static constexpr Stop STOPS[] = {{-5, ha::INDIGO}, {5, ha::BLUE}, {11, ha::CYAN}, {16, ha::GREEN},
                                   {20, ha::AMBER}, {25, ha::ORANGE}, {32, ha::RED}};
  if (!(celsius > STOPS[0].at)) return STOPS[0].color;
  for (size_t i = 1; i < sizeof(STOPS) / sizeof(STOPS[0]); ++i) {
    if (celsius > STOPS[i].at) continue;
    const float share = (celsius - STOPS[i - 1].at) / (STOPS[i].at - STOPS[i - 1].at);
    return mix(STOPS[i].color, STOPS[i - 1].color, static_cast<uint8_t>(share * 255 + 0.5f));
  }
  return STOPS[sizeof(STOPS) / sizeof(STOPS[0]) - 1].color;
}

constexpr uint32_t STATE_OFF = ha::GREY;
inline uint32_t state(uint32_t color) {
  if (!dark) return color;
  if (color == STATE_OFF) return ROLES[OFF].dark;
  if (color == ROLES[HISTORY_OFF].light) return ROLES[HISTORY_OFF].dark;
  if (color == ROLES[HISTORY_EMPTY].light) return ROLES[HISTORY_EMPTY].dark;
  return color;
}
// Relative lightness 0-255 (Rec. 601 weights): enough to tell a colour that sinks into a dark card.
inline uint32_t lightness(uint32_t color) {
  return ((color >> 16 & 0xFF) * 299 + (color >> 8 & 0xFF) * 587 + (color & 0xFF) * 114) / 1000;
}
// A state colour drawn as a glyph, a line or words on the page or a card. Dark lifts the deep ones (indigo, purple)
// until they read on graphite.
inline uint32_t foreground(uint32_t color) {
  color = state(color);
  if (!dark) return color;
  const uint32_t l = lightness(color);
  return l >= 110 ? color : mix(color, 0xFFFFFF, static_cast<uint8_t>(255 - (110 - l) * 3 / 2));
}
// The pale circle behind a tile's icon, a halo, a slider's track: the colour at `amount` over white in light, and
// over the card in dark, where the same share would vanish, a little more.
inline uint32_t tint(uint32_t color, uint8_t amount) {
  color = state(color);
  if (!dark) return mix(color, 0xFFFFFF, amount);
  const unsigned stronger = amount + amount / 3 + 8;
  return mix(color, hex(CARD), static_cast<uint8_t>(stronger > 255 ? 255 : stronger));
}
// A tile's icon in its state colour: a touch darker in light, so a pale bulb still shows; a touch lighter in dark.
inline uint32_t icon(uint32_t color) {
  color = state(color);
  return dark ? mix(foreground(color), 0xFFFFFF, 230) : mix(color, 0x333333, 205);
}
// Vision's icon (local fork): its state colour lifted towards white on its coloured glass while it is on, so it stands
// out of a card of the same hue; a soft white on clear glass.
inline uint32_t vision_icon(uint32_t color, bool lit) { return lit ? mix(state(color), 0xFFFFFF, 140) : VISION_ICON_OFF; }
// A lit Vision card's glass and its hairline (local fork).
inline uint32_t vision_lit(uint32_t color) { return state(color); }
inline uint32_t vision_lit_line(uint32_t color) { return mix(state(color), 0xFFFFFF, 170); }
// Glow's icon (local fork) stands on the card's wash without a circle behind it: deeper in light, so an amber lamp
// still reads on its amber card, and the full lifted colour in dark.
inline uint32_t glow_icon(uint32_t color) {
  color = state(color);
  return dark ? foreground(color) : mix(color, 0x1B1B1B, 185);
}
// The soft area under a graph line: a fifth of the line's colour in light; less over graphite, where the same share
// turns a large brown or purple block.
inline uint8_t fill_opacity() { return dark ? 38 : 51; }
// Keys drawn inside a card of any colour (a wide tile's direct controls): a shade away from the card, towards the
// middle, so they read on white, on a pastel and on graphite alike.
inline uint32_t key_on(uint32_t card, uint8_t depth) {
  return dark ? mix(card, 0xFFFFFF, static_cast<uint8_t>(255 - (255 - depth) * 3 / 2)) : mix(card, 0x000000, depth);
}
// A card under a finger (firmware 0.2.95+): its own colour an eighth of the way to black in light and to white in
// dark, the same share on white, on a pastel and on graphite, so the press reads on every card in both looks.
inline uint32_t pressed(uint32_t card) { return mix(card, dark ? 0xFFFFFF : 0x000000, 224); }
// A calendar's own colour (Home Assistant's, from its registry or the frontend's palette) on the calendar card
// (calendar_card.h): an event's pale block, the words on it, a progress track, and what is past, a step towards the
// card. The same shares as Apple's and the frontend's events: the colour reads, the words stay dark (light in dark).
inline uint32_t calendar_block(uint32_t c) { return dark ? mix(c, hex(CARD), 80) : mix(c, 0xFFFFFF, 58); }
inline uint32_t calendar_ink(uint32_t c) { return dark ? mix(c, 0xFFFFFF, 110) : mix(c, 0x000000, 120); }
inline uint32_t calendar_track(uint32_t c) { return dark ? mix(c, hex(CARD), 90) : mix(c, 0xFFFFFF, 70); }
inline uint32_t calendar_past(uint32_t c) { return mix(c, hex(CARD), 130); }

}  // namespace theme

#ifndef THEME_TEST
#include "lvgl.h"
#include <array>

namespace theme {

inline lv_color_t color(Role role) { return lv_color_hex(hex(role)); }
inline lv_color_t rgb(uint32_t value) { return lv_color_hex(value); }
// The colour an object draws with, as the 24-bit value the table speaks in (what the media card names as the colour
// behind its cover's corners).
inline uint32_t of(lv_color_t value) { return lv_color_to_u32(value) & 0xFFFFFF; }

// ---- paints: shared styles filled from the table
// A board profile gives a widget a paint instead of a colour (`styles: paint_card`). Its style definitions carry no
// colour: the profile's `theme::paints` fills each one with fill(), at boot and after every change of look. The list
// is code, not a table in RAM, and a shared style costs a widget one style reference, about what the local colour it
// replaces cost. The firmware's own long-lived widgets take the same paints through style().
constexpr uint8_t NONE = 0xFF;
enum class Paint : uint8_t {
  page, page_soft, page_pressed, card, raised, key, key_pressed, track, ink, ink_soft, muted, subtle, slate,
  scrim, veil, accent_tint, accent_bright, button_dark, button_dark_pressed, knob, spinner, calibration, camera, COUNT
};
struct PaintRoles { uint8_t bg, border, text, line, arc; };
inline constexpr PaintRoles PAINTS[static_cast<size_t>(Paint::COUNT)] = {
  /* page */                {PAGE, NONE, INK, NONE, NONE},
  /* page_soft */           {PAGE_SOFT, NONE, INK, NONE, NONE},
  /* page_pressed */        {PAGE_PRESSED, NONE, NONE, NONE, NONE},
  /* card */                {CARD, LINE, INK, NONE, NONE},
  /* raised */              {RAISED, RAISED_LINE, INK, NONE, NONE},
  /* key */                 {KEY, NONE, INK, NONE, NONE},
  /* key_pressed */         {KEY_PRESSED, NONE, NONE, NONE, NONE},
  /* track */               {TRACK, NONE, INK, NONE, NONE},
  /* ink */                 {NONE, NONE, INK, NONE, NONE},
  /* ink_soft */            {NONE, NONE, INK_SOFT, NONE, NONE},
  /* muted */               {NONE, NONE, MUTED, NONE, NONE},
  /* subtle */              {NONE, NONE, SUBTLE, NONE, NONE},
  /* slate */               {NONE, SLATE, SLATE, SLATE, NONE},
  /* scrim */               {SCRIM, NONE, NONE, NONE, NONE},
  /* veil */                {VEIL, NONE, NONE, NONE, NONE},
  /* accent_tint */         {ACCENT_TINT, NONE, NONE, NONE, NONE},
  /* accent_bright */       {ACCENT_BRIGHT, NONE, NONE, NONE, NONE},
  /* button_dark */         {BUTTON_DARK, NONE, ON_ACCENT, NONE, NONE},
  /* button_dark_pressed */ {BUTTON_DARK_PRESSED, NONE, NONE, NONE, NONE},
  /* knob */                {KNOB, NONE, NONE, NONE, NONE},
  /* spinner */             {NONE, NONE, NONE, NONE, SPINNER_TRACK},
  /* calibration */         {CALIBRATION_PAGE, NONE, CALIBRATION_INK, NONE, NONE},
  /* camera */              {CAMERA_PAGE, NONE, CAMERA_NOTE, NONE, NONE},
};
// Fill a style with a paint in the current look. Setting a property the style already has allocates nothing.
inline void fill(lv_style_t *style, Paint paint) {
  if (!style) return;
  const PaintRoles &p = PAINTS[static_cast<size_t>(paint)];
  if (p.bg != NONE) lv_style_set_bg_color(style, color(static_cast<Role>(p.bg)));
  if (p.border != NONE) lv_style_set_border_color(style, color(static_cast<Role>(p.border)));
  if (p.text != NONE) lv_style_set_text_color(style, color(static_cast<Role>(p.text)));
  if (p.line != NONE) lv_style_set_line_color(style, color(static_cast<Role>(p.line)));
  if (p.arc != NONE) lv_style_set_arc_color(style, color(static_cast<Role>(p.arc)));
}
// The board profile's styles and the paint of each, as fill() calls. Set and run once in on_boot.
inline void (*paints)() = nullptr;
// The firmware's own shared style for a paint, made on first use.
inline std::array<lv_style_t *, static_cast<size_t>(Paint::COUNT)> own{};
inline lv_style_t *style(Paint paint) {
  auto *&s = own[static_cast<size_t>(paint)];
  if (!s) {
    s = new lv_style_t;
    lv_style_init(s);
    fill(s, paint);
  }
  return s;
}
// What the board draws again after a change of look: the tiles, an open card, the settings page. Set by the profile.
inline void (*redraw)() = nullptr;
// The one way the look changes. Every paint is filled again, what code painted itself follows through redraw(), and the
// glass is drawn again once, whole. A look the screen already has costs nothing, so the setting may be applied on every
// change.
inline void look(bool on, bool force);
// What is drawn dark: Vision always, Soft as its colours are (SoftPalette::dark), the Dark mode setting in the other looks.
inline bool drawn_dark() { return vision || (soft ? soft_colors().dark : dark_wanted); }
// The Dark mode setting. Vision keeps the screen dark and Soft as its colours are whatever it says, and give it back when
// they go.
inline void set_dark(bool on) {
  dark_wanted = on;
  look(drawn_dark(), false);
}
// The Vision look on or off (runtime_tiles::theme_changed): dark at once, and its page and cards drawn again.
inline void set_vision(bool on) {
  if (on == vision) return;
  vision = on;
  look(drawn_dark(), true);
}
// The Soft look on or off, the same way: light at once, its page and cards drawn again.
inline void set_soft(bool on) {
  if (on == soft) return;
  soft = on;
  look(drawn_dark(), true);
}
// Soft's colours (settings_screen::soft_bg), the same way: its page and cards drawn again while it is the look.
inline void set_soft_palette(int at) {
  at = at >= 0 && at < SOFT_PALETTE_COUNT ? at : 0;
  if (at == soft_palette) return;
  soft_palette = at;
  if (soft) look(drawn_dark(), true);
}
// What is drawn: dark or light. `force` repaints even when that stays, for a change of page colour.
inline void look(bool on, bool force) {
  if (on == dark && !force) return;
  dark = on;
  if (paints) paints();
  for (size_t i = 0; i < own.size(); ++i) if (own[i]) fill(own[i], static_cast<Paint>(i));
  // fill() only changes colours, and LVGL reads a colour when it draws: all a new colour asks of a widget is to be drawn
  // again, which is all lv_obj_refresh_style does for one. So no restyle of every widget (lv_obj_report_style_change),
  // which took seconds on a 4 x 6 grid with its pages kept and set off the task watchdog (GitHub #227). And LVGL's
  // invalidation is off while redraw() repaints, as in warm_page: LVGL 9.5 walks the whole tree for blurred widgets at
  // every invalidation, the hidden cards of the kept pages included.
  auto *display = lv_display_get_default();
  if (display) lv_display_enable_invalidation(display, false);
  if (redraw) redraw();
  if (display) {
    lv_display_enable_invalidation(display, true);
    lv_obj_invalidate(lv_display_get_screen_active(display));
  }
}

}  // namespace theme
#endif

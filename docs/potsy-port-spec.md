# Порт Потсика (PotsCharm → hercharm-gadget sdk)

Потсик — наш персонаж (репо владельца), копируем целиком, не переписываем с нуля.
Источник: `/run/media/al/ARCHIVE/code/pusi/firmwares/PotsCharm`
- `src/potsy.cpp` (9399 строк), `include/potsy.h` (786)
- цели: процедурный персонаж на TFT_eSPI; анимации — синус от `now` (мс), настроение Mood, эмоты.

Целевая прошивка: этот репо, `sdk/firmware/` (ESP-IDF, не Arduino).
Плата: `xiaozhi-cube-154` (ST7789 240x240 SPI, RGB565 фреймбуфер).
Сборка/прошивка: `pio run -e xiaozhi-cube-154 [-t upload --upload-port /dev/ttyACM0]` из `sdk/firmware/esp32/`.
Критерий готовности: сборка SUCCESS, Потсик на главном (hero) экране вместо 1-бит маскота Nous girl,
анимируется (idle bob, blink), говорит ртом при m.speaking, реплика показывается баблом Потсика.

## Стратегия: compat-слой, а не переписывание

Пишем `sdk/firmware/core/src/potsy/compat.hpp/.cpp` — класс `TFT_eSPI`-шим над `hg::Canvas`
(firmware/core/include/hg/canvas.hpp): canvas RGB565, побандовый рендер (base_y/rows, set_clip_rows).
Шим хранит `hg::Canvas* c` и делегирует. Координаты — полные (canvas бэнда держит полную систему координат).

Методы, реально используемые potsy.cpp (частоты):
fillRect(97) fillRoundRect(95) fillTriangle(87) fillCircle(84) drawLine(30) drawFastHLine(27)
fillEllipse(26) width(19) drawPixel(11) drawFastVLine(11) setTextSize(9) setCursor(8) setTextColor(7)
print(7) drawCircle(7) textWidth(6) drawRoundRect(6) setTextWrap(5) readPixel(4) height(3)
drawEllipse(3) drawRect(2) color16to8(2) drawWideLine(1) color565(1)
Реализация: fillEllipse/drawEllipse — по X^2/a^2+Y^2/b^2<=1; drawWideLine — капсулы (2 круга + прямоугольник,
упрощённо); readPixel — добавить публичный `uint16_t read_pixel(x,y)` в hg::Canvas (возвращает 0 вне);
текст: font 5x7 SDK (canvas.text / font::), textWidth → font::width*size.

## Стабы-хедеры (в sdk/firmware/core/src/potsy/)

- `theme_mini.h`: константы цветов из PotsCharm include/theme.h (только используемые potsy:
  BG, BLACK, WHITE, CYAN, PINK, AMBER, VAPOR_BLUE, VAPOR_PINK, W95_LIGHT и др. — взять значения из
  исходного theme.h); функции: blend(a,b,t) — линейный; wrapText/wrapTextRU → hg::wrap_text;
  printRU/textWidthRU → обычный шрифт (v1 без кириллицы-спецшрифта; строки RU придут из potsy_ru.inc —
  если font5x7 не имеет кириллицы, можно пока выводить EN-пул: isRU() в стабе настроек вернуть false);
  speech* — метрики от font5x7; tr(s) → s; showToast/drawWing/colorFor/consume*/setRedGlyph — no-op.
- `state_mini.h`: enum DetectionType — СКОПИРОВАТЬ список значений из PotsCharm include/state.h
  (potsy использует AIRTAG, ALPR, UNKNOWN и др. — взять enum целиком).
- `sound_stub.h`: `Sound::play(Id)` — no-op (enum Id с THROW/LAND копией).
- `clock_stub.h`: Clock::isSet/trusted/night/weekend/weekday/hour/localDay/greetedDay/setGreetedDay —
  на базе `hg::WallTime` (App получает его из порта; в стабе можно time()/localtime).
- `dex_stub.h`: Dex::rarity(dt) → Rarity::COMMON.
- пустые: signatures.h, void_eye.h, draw_band.h, fast_sprite.h (FastSprite* — nullptr-путь:
  в potsy.cpp:6351 `g_frameSprite` проверяется — оставить extern-переменную, инициализировать nullptr).
- `settings_stub.h`: если potsy.cpp зовёт Settings:: — заглушить нужное; isRU() → false (EN-пул).

## Копия

`src/potsy.cpp` → `sdk/firmware/core/src/potsy/potsy.cpp` (правки только include-путей и
ардуино-зависимостей: millis()/random()/Preferences). millis() → esp_timer_get_time()/1000 (ESP-IDF
доступен) или параметр now, который tick() уже принимает. Preferences (s_petPrefs) — заглушка NVS no-op.
random(a,b) — свой PRNG (нelегкий, srand не нужен).

## Интеграция (минимальные правки core)

1. `firmware/core/include/hg/ui.hpp` / `ui.cpp`:
   - UiModel добавить `uint32_t now_ms` (App::update_model заполняет своим now()*1000 — App::now()
     возвращает мс? проверить: app.cpp now() — мс аптайма) и флаг `bool potsy = true;` (v1 константа).
   - В `Ui::draw_hero` (ui.cpp:1942): если potsy — вместо draw_face(...) вызвать
     `potsy::tick(shim_canvas, mx + size/2, my, size, m.now_ms, true, 0.5f)` с talking=m.speaking;
     анимационные банды: hero_anim_rows оставить как есть (v1: полный redraw hero-банда каждый кадр
     допустим — проверить, что перерисовка бэндов не рвёт анимацию; проще отключить hash-пропуск
     для hero-бэнда при potsy).
   - Potsy::tick рисует и пузырь — для реплики: в Ready/Responding передавать текст reply в
     Potsy::say() (проверить API в potsy.h: есть announce/say/bubble функции) вместо caption.
2. `firmware/esp32/main/CMakeLists.txt`: добавить файлы potsy/.
3. Протокол/плагин не трогать.

## Ограничения

- Flash: занято 85.7% (ota_0 1984 KB, свободно ~14%). potsy.cpp добавит ~100-200 KB кода.
  Если не влезает: отключить тяжёлое (питомцев, эмоты-скрипты) через стабы, но тело/настроения/пузырь
  сохранить. SIZE-оптимизация уже включена (CONFIG_COMPILER_OPTIMIZATION_SIZE).
- RAM: рисование каждый кадр — potsy тратил ~7 мс на TFT_eSPI; здесь Canvas в PSRAM — проверить
  FPS по логам (frame_stats), если медленно — рисовать Потсика только на изменении.
- Ничего в протоколе ws/плагине не менять. Кнопки/подключение/аудио — как работали.
- Тесты firmware/tests (host) не должны сломаться: правки Canvas (read_pixel) — добавить юнит-тест
  по образцу соседних.

## Порядок работы

1. compat + стабы → 2. копия potsy + компиляция-цикл до SUCCESS → 3. интеграция draw_hero →
4. сборка + (если устройство на /dev/ttyACM0) прошивка и серийная проверка `hg.ui`/frame_stats →
5. коммит: `feat(ui): port Potsy mascot from PotsCharm (compat layer over hg::Canvas)`.

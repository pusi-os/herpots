# Состояние прошивки HERPOTS — 2026-11-23

Снимок рабочего состояния обеих плат. Всё ниже проверено на живом железе; сборки зелёные.

## База

- Проект: `/run/media/al/ARCHIVE/code/hercharm-gadget`, ветка `master`.
- Основа — форк `hermes-gadget-sdk` (Adolanium, MIT). Ядро в `sdk/firmware/core`, платформенный слой в `sdk/firmware/esp32`.
- Коммиты: `ba73233` (базовый SDK 0.6.0 + xiaozhi-cube-154), `c6ef5e7` (порт Потсика). Правки UI/i18n/батареи/плат после этого — рабочее дерево, не закоммичены.
- Версия прошивки: `hermes_gadget 0.6.0`, ESP-IDF 6.1.0, PlatformIO core 6.2.0, платформа `platformio/espressif32@8.0.0`.

## Оболочка: что изменено относительно стока

| Область | Файл | Изменение |
|---|---|---|
| Язык | `core/src/i18n_ru.cpp` (288 строк), `core/src/i18n.cpp`, `include/hg/i18n.hpp` | Русский UI; испанский удалён полностью (`i18n_es.cpp` удалён, `Lang::{En,Ru}`) |
| Персонаж | `core/src/potsy/*` (potsy.cpp, potsy_ru.inc, glue.cpp, settings.h) | Потсик из PotsCharm перенесён compat-слоем поверх `hg::Canvas`; русские реплики (~120 строк), `MIN_BUBBLE_MS=7000`, `OUTFIT_RU` = 20 элементов |
| Шапка hero | `core/src/ui.cpp` | Слева иконка батареи (4 сегмента, цвет по уровню, носик, молния при зарядке), справа часы; индикатор онлайн — только точка |
| Потсик | `core/src/ui.cpp` `draw_hero` | `drawn = size`, смещение `my + size - drawn + 6*s` — ноги выше подписи |
| Время | `esp32/main/port_time.cpp`, `main.cpp` | TZ по умолчанию `MSK-3`, SNTP всегда включён |
| Звук | `esp32/main/port_audio.cpp` | PCM16 (`opus_bitrate: 0` в конфиге гейтвея), прайм-буфер `kPrimeMs=180`, `kDrainSettleMs=100` |
| Flash | `core/src/mascot.cpp`, `core/src/mascot_data.cpp` | Маскот 64/192 px удалён, `kSizes = {96,144}` — уложились в OTA-слот |
| Батарея (дефолты) | `core/src/app.cpp` | `battery_display="icon"`, `battery_estimate="liion"` |

## Плата: кубик (`xiaozhi-cube-154`)

- ESP32-S3-N16R8, 16 МБ flash, 8 МБ octal PSRAM. USB-JTAG `/dev/ttyACM0`. ID устройства `hg-2eae81f02103f92e`.
- ST7789 SPI 240×240: MOSI 10, SCLK 9, CS 14, DC 8, RST 18, BL 13 (PWM), инверсия включена. Ориентация — без поворота (swap/mirror выключены).
- Кнопки: talk = BOOT0, cancel = 40, VOL− = 39. Питание: latch GPIO21, заряд-детект GPIO38, батарея ADC GPIO17, `mv_ratio=2`.
- Аудио: mic I2S sck 5 / ws 4 / sd 6; speaker bclk 15 / ws 16 / dout 7.
- Сборка/прошивка:
  ```
  cd sdk/firmware/esp32
  pio run -e xiaozhi-cube-154 -t upload --upload-port /dev/ttyACM0
  ```
- Размер: RAM 26.3% (86 176 / 327 680), Flash 88.8% (1 804 423 / 2 031 616; `firmware.bin` 1 804 848 Б). Свободно ~11% OTA-слота.

## Плата: zhengchen (`zhengchen-154`)

- ESP32-S3, 16 МБ flash, octal PSRAM 80 МГц. UART через CH340 `/dev/ttyUSB0`. ID в гейтвее `hg-e09a8e1cf6d9b695`, сервер `ws://192.168.1.161:8765/gadget` (compy), pairing одобрен.
- ST7789 SPI 240×240: MOSI 41, SCLK 42, CS 21, DC 40, RST 45, BL 20 (PWM), инверсия включена.
- **Ориентация (подобрана на железе):** `swap_xy=true, mirror_x=false, mirror_y=true`, `gap_x=80, gap_y=0`. Итог: картинка целиком на экране, кнопки сверху — так удобнее владельцу.
  - Ловушки, проверенные на этой панели: `MX|MY` (rot 2 по таблице PotsWatch) → вверх ногами; `MX|MY|MV` → зеркало + обрезано справа; `swap_xy+MX` → зеркало + срез правого края; `gap_y=80` → сдвиг ВНИЗ, а не вправо (при swap осей отступ уходит в X только через `gap_x`).
- Кнопки: talk = BOOT0, cancel = VOL+ (GPIO10), VOL− = 39. Питание: rail hold GPIO2, заряд-детект GPIO9 (active high), батарея ADC GPIO8 (ADC1_CH7), `mv_ratio=2`.
- Аудио: то же трио, что у кубика (mic 5/4/6, speaker 15/16/7).
- Прошивка — ТОЛЬКО вручную в BOOT-режиме, одним вызовом esptool, без промежуточных reset:
  ```
  cd sdk/firmware/esp32 && pio run -e zhengchen-154
  /home/al/.platformio/penv/bin/esptool.py --chip esp32s3 --port /dev/ttyUSB0 \
    --baud 460800 --before usb-reset --after hard-reset write-flash -z \
    --flash-mode dio --flash-size 16MB --flash-freq 80m \
    0x20000 .pio/build/zhengchen-154/firmware.bin
  ```
  Смещения (hermes-gadget, НЕ PotsWatch): bootloader `0x0`, partitions `0x8000`, app `0x20000`. `0x10000` ломает загрузку («no bootable app partitions»).
- Настройка по UART-консоли (115200, после прошивки): `set wifi_ssid <ssid>`, `set wifi_pass <pass>`, `set server ws://192.168.1.161:8765/gadget`, проверка `get server`. Порт надо открывать с `setDTR(False); setRTS(False)` и паузой ~0.5 с, иначе первые команды теряются в мусоре буфера.
- Размер: flash 88.8%, `firmware.bin` 1 817 568 Б.

## Изменение 2026-11-23 (после снимка): баблы Потсика

`core/src/potsy/glue.cpp` + `ui.cpp`: внешний feed бабла убран. Раньше в бабл над Потсиком
шло `m.body` (эхо распознанного в Thinking, `reply_` агента в Responding) — оно мелькало
на каждом чанке стрима. Теперь бабл несёт ТОЛЬКО его собственную болтовню (пулы potsy_ru,
idle-машина Потсика); распознанный запрос и ответ агента остаются в caption-полосе под ним
и на текстовых экранах. Подпись `draw_hero` без параметра bubble.

## Известные ограничения

- Треск в озвучке: снижен прайм-буфером и PCM16, окончательно не подтверждён на слух после последней сборки.
- Поворот 90° на zhengchen не решён: панель неквадратная по поведению, MADCTL-комбинации вне таблицы дают срез. Текущая ориентация принята владельцем как удобная; дальнейшая работа — через offset в драйвере панели.
- Правки не закоммичены в git (рабочее дерево поверх `c6ef5e7`).

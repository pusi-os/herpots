# HerPots

Голосовой компаньон для [Hermes Agent](https://github.com/NousResearch/hermes-agent): маленькое
ESP32-S3 устройство с экраном, микрофоном и динамиком, персонаж **Потсик**, русский интерфейс.

Держишь кнопку — говоришь — слышишь ответ. Все инструменты, память и скиллы Hermes на устройстве.

```
herpots/
├── sdk/firmware/  прошивка (форк hermes-gadget-sdk, ESP-IDF + PlatformIO)
├── plugin/        плагин Hermes (Python, kind: platform)
└── docs/          состояние прошивки, план публикации
```

## Что внутри

**Прошивка.** Форк [hermes-gadget-sdk](https://github.com/Adolanium/hermes-gadget-sdk) (MIT) с:
- русским UI (288 строк, `core/src/i18n_ru.cpp`; испанский удалён);
- персонажем Потсик, перенесённым из PotsCharm через compat-слой поверх `hg::Canvas`;
- двумя платами: `xiaozhi-cube-154` (кубик) и `zhengchen-154`;
- 4-сегментным индикатором батареи, часами по МСК, баблами на 7 секунд;
- PCM16 аудио-трактом с прайм-буфером 180 мс.

**Плагин.** `kind: platform`, держит WebSocket на гейтвее (`/gadget`, порт 8765), регистрирует
8 инструментов (`gadget_devices`, `gadget_display`, `gadget_ask`, `gadget_speak`, `gadget_action`,
`gadget_play`, `gadget_widget`, `gadget_interpret`), веб-дашборд и CLI-команды.

Имя платформы и инструментов остаётся `gadget` намеренно: устройства, прошитые родительской
прошивкой, и конфиги с `platforms.gadget` продолжают работать без правок.

## Установка

```bash
hermes plugins install pusi-os/herpots --subdir plugin
```

Или вручную: скопировать `plugin/` в `~/.hermes/plugins/herpots/`, затем
`hermes config set platforms.gadget.enabled true`.

## Прошивка

Кубик (`xiaozhi-cube-154`), USB-JTAG:

```bash
cd sdk/firmware/esp32
pio run -e xiaozhi-cube-154 -t upload --upload-port /dev/ttyACM0
```

Zhengchen (`zhengchen-154`) — только в BOOT-режиме, одним вызовом esptool:

```bash
cd sdk/firmware/esp32 && pio run -e zhengchen-154
/home/al/.platformio/penv/bin/esptool.py --chip esp32s3 --port /dev/ttyUSB0 \
  --baud 460800 --before usb-reset --after hard-reset write-flash -z \
  --flash-mode dio --flash-size 16MB --flash-freq 80m \
  0x20000 .pio/build/zhengchen-154/firmware.bin
```

Смещения: bootloader `0x0`, partitions `0x8000`, app `0x20000`. Подробности и ловушки —
`docs/firmware-state-2026-11-23.md`.

## Язык

Русский — язык по умолчанию (решение владельца 2026-11-23). Английский доступен как второй
язык в меню устройства.

## Лицензия

GPL-3.0-or-later. Персонаж Потсик портирован из PotsCharm (GPL-3.0); прошивочная основа —
hermes-gadget-sdk (MIT), что совместимо с GPL. Атрибуция — в `NOTICE`.

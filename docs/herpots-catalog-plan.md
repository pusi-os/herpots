# HERPOTS — план публикации в каталоге плагинов Hermes

Дата: 2026-11-23. Статус: план, ждёт подтверждения владельца перед заведением карт в канбане.

## 1. Что публикуем

Плагин HERPOTS = текущий `~/.hermes/plugins/gadget` (kind: `platform`, 0.6.0, ~8 800 строк Python)
с брендингом Потсика и поддержкой двух плат (кубик, zhengchen) в прошивке-форке sdk.

Каталог — только про плагин; прошивка живёт в том же репо и ставится с платы/через веб-установщик.

## 2. Результат локального аудита (факты, не оценки)

```
hermes plugins validate ~/.hermes/plugins/gadget
→ Validation passed.  16 проверок ✓
```

Проходят: манифест, `requires_hermes`, capabilities против фактических регистраций, security scan
(`safe`), отсутствие переопределения ядра, desktop surface внутри SDK.

**Одна проблема уровня «ревьюер будет против»:**

⚠ **private core imports — 19 модулей Hermes напрямую.** Правило 9 каталога (public surfaces only)
формально нарушено. Полный список:

| Модуль | Где |
|---|---|
| `agent.i18n` | adapter.py:51 |
| `agent.secret_scope` | — |
| `gateway`, `gateway.config`, `gateway.pairing` | cli.py:29 |
| `gateway.platforms.base` | — |
| `gateway.platforms.base_exec_approval` | compat.py:96 |
| `gateway.platforms.event` | — |
| `gateway.platforms._shared` | — |
| `gateway.session_context` | — |
| `hermes_cli.config`, `hermes_cli.profiles`, `hermes_cli.setup` | adapter.py:438, setup.py:18 |
| `hermes_constants` | — |
| `tools`, `tools.approval`, `tools.transcription_tools`, `tools.tts_tool`, `tools.url_safety` | adapter.py:1297-1298, playback.py:427 |

Это не ошибка наша: плагин `kind: platform` по определению живёт в процессе гейтвея и опирается на
`gateway.platforms.base` (базовый класс адаптера платформы). Но каталог ждёт, что такие швы
закрыты публичным API (`ctx.*`). Пути:

- **(a)** Открыть issue в hermes-agent на недостающие швы (`register_platform` уже публичный —
  нужны базовый класс адаптера, `approval_timeout_seconds`, `transcribe_audio`,
  `text_to_speech_tool`, `redirect_target_from_response`). ~5 швов, остальное — константы.
- **(b)** Принять ⚠ как известное ограничение: `caution`-находки не блокируют мерж, ревьюер читает их
  и решает. Риск — отказ по правилу 3/9.
- **(c)** Минимум: свести к абсолютно необходимому (5 швов выше), остальные 14 импортов заменить
  на `ctx.*` или локальные копии констант.

Рекомендация: (c) + (a) параллельно. Это отдельная карта работ, не блокирует остальное.

## 3. Блокеры и их статус (обновлено 2026-11-23)

**3.1 OTA — НЕ self-updater (пересмотрено).**
`ota.py` + `releases.py` шьют прошивку ESP32-УСТРОЙСТВА по WebSocket (стейджинг образа,
OTA-слот устройства). Файлы самого плагина они не трогают — правило 3 каталога (no
self-updating code) о плагинах, не о контенте для стороннего железа. Аналог: плагин
Home Assistant тоже рассылает прошивки на устройства. Статус: остаётся в плагине,
в PR честно раскрывается в description («firmware staging for the companion devices,
never touches plugin files; updates flow through reviewed SHA pins, not in-app updaters»).
Отдельно: `hermes gadget update` скачивает релизы с GitHub по зафиксированному тегу —
это «download pinned by checksum», разрешённая схема.

**3.2 Лицензия — решено: GPL-3.0-or-later.**
SDK — MIT, PotsCharm (Потсик) — GPL-3.0. Порт персонажа внутри → **вся сборка становится GPL-3.0**.
Это законно (MIT совместим с GPL), но должно быть объявлено: `license: GPL-3.0-or-later` в
`plugin.yaml`, `LICENSE` в корне, раздел в README. Отдельно: в каталоге есть GPL-плагины, но
ревьюер спросит про совместимость — ответ готов: MIT-часть от Adolanium, персонаж и юиксы наши.

**3.3 Публичный репозиторий — решено: https://github.com/pusi-os/herpots** (public, main).
Локально `/run/media/al/ARCHIVE/code/herpots`; ветка sdk в `sdk/`, плагин в `plugin/`.

**3.4 English-first — решено владельцем: русский остаётся дефолтом.** В PR указывается
прямо: the device UI ships Russian-first by owner choice; English is available in the
device menu. Catalog sweeps expect English-first UI — это предупреждение ревьюеру, не
автоотказ; если попросят — сделаем English-дефолт одной строкой (оба языка в прошивке есть).

## 4. Работы по разбивке (кандидаты в карты канбана)

| # | Карта | Оценка |
|---|---|---|
| E1 | Публичный GitHub-репо, пуш рабочего дерева, LICENSE GPL-3.0, README (ru+en) | S |
| E2 | `plugin.yaml`: брендинг HERPOTS, license, author, docs_url, скриншоты | S |
| E3 | Вырезать OTA/self-updater из каталожной сборки (4 подпункта §3.1) | M |
| E4 | Свести private imports: 5 швов → ctx.*, 14 констант локально | M |
| E5 | Issue в hermes-agent на недостающие публичные швы | S |
| E6 | English-first: дефолт языка для публичной сборки, реплики Потсика EN | M |
| E7 | Прошивочная часть: `boards/zhengchen-154` + `boards/xiaozhi-cube-154` в репо, документация прошивки | S |
| E8 | `hermes plugins validate` зелёный после всех правок, разбор warnings | S |
| E9 | PR: `plugin-catalog/herpots.yaml` с 40-hex SHA, описание, скриншоты | S |
| E10 | Ответ на ревью, доработки | ? |

Порядок: E1 → E2 → E3 → E4/E5 параллельно → E6 → E7 → E8 → E9 → E10.

## 5. Открытые вопросы владельцу (нужны до старта)

1. **Имя плагина в каталоге:** `herpots` (новое имя, чистый брендинг) или `gadget`
   (совместимость с уже прошитыми устройствами и `platforms.gadget` в конфигах).
   Переименование пакета затрагивает `~/.hermes/config.yaml` (`platforms.gadget`),
   конфиг на compy и ID каналов — минимум 2 живых инсталляции.
2. **Репо:** новый `pusi-os/herpots` или форк `Adolanium/hermes-gadget-sdk` с мержем апстрима?
   Форк сохраняет историю и MIT-атрибуцию, новый репо чище для брендинга.
3. **English-first:** делаем английский дефолтом для публичной версии или идём с русским?
4. **Скоуп ревью private imports:** идём в (c) — урезаем до 5 швов, или (b) — публикуем с ⚠?

## 6. Что НЕ входит

- Изменение протокола устройства (ломает прошитые устройства).
- Публикация самой прошивки в каталог (каталог только про плагины).
- Смена WS-порта 8765 и путей `/gadget`.

# Attribution / NOTICE

HerPots combines work from several sources. This file records what came from where.

## hermes-gadget-sdk — MIT

- Upstream: https://github.com/Adolanium/hermes-gadget-sdk
- Base of `sdk/` (firmware) and of `plugin/`, pinned at upstream commit `35dbae7`
  (catalog pin of the installed copy was `da48d257`, the fork starts from the newer tree).
- Copyright (c) 2026 Hermes Gadget SDK contributors.
- Full text: `sdk/LICENSE`, `sdk/LICENSES/`.

## PotsCharm — GPL-3.0

- Character **Potsy** (Потсик): artwork atlases, animation timing, speech pools, outfit data.
- Ported into `sdk/firmware/core/src/potsy/` through a compatibility layer over `hg::Canvas`;
  the character code is carried over as-is rather than rewritten.
- The speech pools were extended with Russian lines (`potsy_ru.inc`).
- Because these GPL-3.0 components are distributed inside HerPots, the combined work is
  licensed GPL-3.0-or-later. The MIT-licensed parts above remain MIT on their own.

## Layout notes

- `sdk/firmware/esp32/boards/xiaozhi-cube-154/` — cube board definition.
- `sdk/firmware/esp32/boards/zhengchen-154/` — zhengchen board definition (added by this project;
  pin map cross-checked against ComPiXi `boards/zhengchen/1.54tft-wifi/config.h`).

## Security scanning

`hermes plugins validate` at 0.7.0 reports `security scan — safe`, no core overrides, and
desktop surface inside the plugin SDK. Remaining warnings are the platform adapter's private
imports of gateway internals (`gateway.platforms.base`, `agent.i18n`, `tools.tts_tool`), which
`kind: platform` plugins need for their seam into the gateway; see `docs/herpots-catalog-plan.md`.

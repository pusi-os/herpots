"""Shared fixtures for the HerPots plugin's own tests (the root ``plugin/`` half).

The SDK's suites (``sdk/tests``) cover the upstream ``sdk/plugin`` copy. These cover what this
repository changed, so they import the root plugin as ``hermes_gadget_plugin`` — the name its
modules use for sibling imports.
"""

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PLUGIN = ROOT / "plugin"


def _ensure_plugin_package() -> None:
    if "hermes_gadget_plugin" in sys.modules:
        return
    spec = importlib.util.spec_from_file_location(
        "hermes_gadget_plugin", PLUGIN / "__init__.py", submodule_search_locations=[str(PLUGIN)])
    module = importlib.util.module_from_spec(spec)
    sys.modules["hermes_gadget_plugin"] = module
    spec.loader.exec_module(module)


_ensure_plugin_package()

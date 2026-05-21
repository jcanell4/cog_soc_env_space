"""Tests for DeathBiomassOrder."""

from __future__ import annotations

import random
import sys
import unittest
from pathlib import Path

_ROOT = Path(__file__).resolve().parents[3]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from tools.generator.orders import (
    BinSizeMap,
    DeathBiomassOrder,
    FromContext,
    GenerationContext,
    Partial,
    Proportion,
)


class TestDeathBiomassOrder(unittest.TestCase):
    def test_resolve_from_context_and_partial(self) -> None:
        ctx = GenerationContext(rng=random.Random(1), n_bins=4, n_stages=3)
        order = DeathBiomassOrder(
            bins=FromContext("n_bins"),
            size=BinSizeMap(Partial({1: 0, 2: 1})),
            fraction=Proportion(Partial({0: {0: 0.6}})),
        )
        order.apply_subdefaults()
        cfg = order.resolve(ctx)
        self.assertEqual(cfg["bins"], 4)
        self.assertEqual(cfg["size"], {1: 0, 2: 1})
        self.assertEqual(cfg["fraction"], {0: {0: 0.6}})


if __name__ == "__main__":
    unittest.main()

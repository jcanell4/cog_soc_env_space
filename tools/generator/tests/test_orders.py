"""Unit tests for generation orders."""

from __future__ import annotations

import random
import unittest

import sys
from pathlib import Path

_ROOT = Path(__file__).resolve().parents[3]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from tools.generator.orders import (
    DefaultOrder,
    GenerationContext,
    Literal,
    NutrientsFromTier,
    ProportionalDecay,
)
from tools.generator.models.niche_defaults import NicheGenDefaultCatalog
from tools.generator.models.niche_process import NicheGenProcess, generate_niche_from_process


class TestOrders(unittest.TestCase):
    def test_literal(self) -> None:
        ctx = GenerationContext(rng=random.Random(0))
        self.assertEqual(Literal(42.0).resolve(ctx), 42.0)

    def test_default_order_with_catalog(self) -> None:
        ctx = GenerationContext(rng=random.Random(1), stochastic=False, surface=1000.0)
        order = DefaultOrder()
        order.set_defaults(NicheGenDefaultCatalog.nutrients, key="interval_tier")
        v = order.resolve(ctx)
        self.assertGreater(v, 0.0)

    def test_niche_process_minimal(self) -> None:
        p = NicheGenProcess(surface=500.0, n_bins=3, stochastic=False)
        niche = generate_niche_from_process(p, rng=random.Random(2))
        self.assertEqual(niche.surface, 500.0)
        self.assertEqual(len(niche.return_rate), 3)


if __name__ == "__main__":
    unittest.main()

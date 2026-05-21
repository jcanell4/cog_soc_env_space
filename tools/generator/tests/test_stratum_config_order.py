"""Tests for StratumConfigOrder."""

from __future__ import annotations

import random
import sys
import unittest
from pathlib import Path

_ROOT = Path(__file__).resolve().parents[3]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from tools.generator.orders import GenerationContext, Partial, SlipLastValue, StageHeightMap
from tools.generator.orders.scalar import Literal
from tools.generator.orders.stratum import StratumConfigOrder


class TestStratumConfigOrder(unittest.TestCase):
    def test_slip_last_value_extends_to_n_stages(self) -> None:
        ctx = GenerationContext(rng=random.Random(2), n_stages=4)
        slip = SlipLastValue(Partial({0: 0, 1: 0, 2: 1}))
        heights = slip.resolve(ctx)
        self.assertEqual(len(heights), 4)
        self.assertEqual(heights, [0, 0, 1, 1])

    def test_stratum_config_order(self) -> None:
        ctx = GenerationContext(rng=random.Random(3), n_stages=3)
        order = StratumConfigOrder(
            n_strata=Literal(2),
            stage_height_class=StageHeightMap(SlipLastValue(Partial({0: 0, 1: 0, 2: 1}))),
        )
        order.apply_subdefaults()
        cfg = order.resolve(ctx)
        self.assertEqual(cfg["n_strata"], 2)
        self.assertEqual(cfg["stage_height_class"], [0, 0, 1])


if __name__ == "__main__":
    unittest.main()

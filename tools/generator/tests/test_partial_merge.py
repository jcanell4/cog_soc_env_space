"""Tests for Partial and deep_merge."""

from __future__ import annotations

import random
import sys
import unittest
from pathlib import Path

_ROOT = Path(__file__).resolve().parents[3]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from tools.generator.orders import GenerationContext, Partial
from tools.generator.orders.util import deep_merge


class TestPartialMerge(unittest.TestCase):
    def test_deep_merge_nested(self) -> None:
        base = {0: {0: 0.1}}
        over = {0: {1: 0.5}, 1: {0: 0.2}}
        merged = deep_merge(base, over)
        self.assertEqual(merged[0][0], 0.1)
        self.assertEqual(merged[0][1], 0.5)
        self.assertEqual(merged[1][0], 0.2)

    def test_partial_dict_override(self) -> None:
        ctx = GenerationContext(rng=random.Random(0))
        order = Partial({1: 0, 2: 1}, default={})
        self.assertEqual(order.resolve(ctx), {1: 0, 2: 1})


if __name__ == "__main__":
    unittest.main()

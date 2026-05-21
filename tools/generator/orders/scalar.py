# """Scalar-valued generation orders."""

# from __future__ import annotations

# from dataclasses import dataclass

# from .base import Order
# from .context import GenerationContext


# @dataclass(frozen=True)
# class Literal(Order[float]):
#     value: float

#     def resolve(self, ctx: GenerationContext) -> float:
#         return float(self.value)


# @dataclass(frozen=True)
# class Gaussian(Order[float]):
#     mean: float
#     std: float
#     floor: float = 0.0

#     def resolve(self, ctx: GenerationContext) -> float:
#         if ctx.stochastic:
#             v = self.mean + self.std * ctx.rng.gauss(0.0, 1.0)
#         else:
#             v = self.mean
#         return float(max(self.floor, v))


# @dataclass(frozen=True)
# class UniformRange(Order[float]):
#     low: float
#     high: float

#     def resolve(self, ctx: GenerationContext) -> float:
#         lo, hi = float(self.low), float(self.high)
#         if lo > hi:
#             lo, hi = hi, lo
#         if ctx.stochastic:
#             return float(ctx.rng.uniform(lo, hi))
#         return float(0.5 * (lo + hi))


# @dataclass(frozen=True)
# class NutrientsFromTier(Order[float]):
#     """Map tier 0..4 to interval in [0, 1000 * surface] (niche nutrients)."""

#     tier: int
#     std_frac: float = 0.28

#     def resolve(self, ctx: GenerationContext) -> float:
#         cap = 1000.0 * max(0.0, float(ctx.surface))
#         if cap <= 0.0:
#             return 0.0
#         span = cap / 5.0
#         k = max(0, min(4, int(self.tier)))
#         lo, hi = k * span, (k + 1) * span
#         mid = 0.5 * (lo + hi)
#         width = max(hi - lo, 1e-12)
#         std = width * self.std_frac
#         if ctx.stochastic:
#             v = mid + std * ctx.rng.gauss(0.0, 1.0)
#         else:
#             v = mid
#         return float(max(0.0, min(cap, v)))


# @dataclass(frozen=True)
# class ProspectingSharpness(Order[float]):
#     mean: float = 0.85
#     std: float = 0.3

#     def resolve(self, ctx: GenerationContext) -> float:
#         if self.std < 0:
#             raise ValueError("prospecting std must be non-negative")
#         if ctx.stochastic:
#             v = self.mean + self.std * ctx.rng.gauss(0.0, 1.0)
#         else:
#             v = self.mean
#         return float(max(0.0, v))

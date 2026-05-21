# """Generation process for :class:`NicheGen` (typed orders + defaults)."""

# from __future__ import annotations

# import random
# from dataclasses import dataclass, field
# from typing import List, Optional, Union

# from ..orders import DefaultOrder, GenerationContext, Order, apply_field_defaults
# from ..orders.base import Order as OrderBase
# from ..orders.scalar import Literal
# from .niche import NicheGen
# from .niche_defaults import NicheGenDefaultCatalog


# def _coerce_order(value: Union[Order, float, int, None], default: Order) -> Order:
#     if value is None:
#         return default
#     if isinstance(value, OrderBase):
#         return value
#     return Literal(float(value))


# @dataclass
# class NicheGenProcess:
#     """Describe how to build a :class:`NicheGen` via orders (minimal config friendly)."""

#     surface: Union[Order[float], float] = 1000.0
#     n_bins: Union[Order[float], int, float] = 3
#     n_factors: Union[Order[float], int, float] = field(default_factory=lambda: DefaultOrder())
#     nutrients: Order[float] = field(default_factory=lambda: DefaultOrder())
#     return_rate: Order[List[float]] = field(default_factory=lambda: DefaultOrder())
#     conditions: Order[List[float]] = field(default_factory=lambda: DefaultOrder())
#     limiting_factors: Order[List[float]] = field(default_factory=lambda: DefaultOrder())
#     prospecting_scan_sharpness: Order[float] = field(default_factory=lambda: DefaultOrder())
#     stochastic: bool = False
#     _defaults_applied: bool = field(default=False, repr=False)

#     def apply_defaults(self, catalog: type[NicheGenDefaultCatalog] = NicheGenDefaultCatalog) -> None:
#         nf = _coerce_order(self.n_factors, DefaultOrder())
#         if isinstance(nf, DefaultOrder):
#             nf.set_defaults(catalog.n_factors)
#         apply_field_defaults(
#             {
#                 "nutrients": self.nutrients,
#                 "return_rate": self.return_rate,
#                 "conditions": self.conditions,
#                 "limiting_factors": self.limiting_factors,
#                 "prospecting_scan_sharpness": self.prospecting_scan_sharpness,
#             },
#             catalog.field_catalogs(),
#         )
#         self._defaults_applied = True


# def generate_niche_from_process(
#     process: NicheGenProcess,
#     *,
#     rng: Optional[random.Random] = None,
# ) -> NicheGen:
#     if not process._defaults_applied:
#         process.apply_defaults()

#     r = rng if rng is not None else random.Random()
#     surface_o = _coerce_order(process.surface, Literal(1000.0))
#     n_bins_o = _coerce_order(process.n_bins, Literal(3.0))
#     n_factors_o = _coerce_order(process.n_factors, DefaultOrder())

#     surface = surface_o.resolve(GenerationContext(rng=r, stochastic=process.stochastic))
#     if surface < 0:
#         raise ValueError("surface must be non-negative")

#     n_bins = int(n_bins_o.resolve(GenerationContext(rng=r, stochastic=process.stochastic, surface=surface)))
#     if n_bins < 1:
#         raise ValueError("n_bins must be >= 1")

#     ctx = GenerationContext(
#         rng=r,
#         stochastic=process.stochastic,
#         surface=surface,
#         n_bins=n_bins,
#     )
#     n_factors = int(n_factors_o.resolve(ctx))
#     if n_factors < 1:
#         raise ValueError("n_factors must be >= 1")
#     ctx.n_factors = n_factors
#     ctx.resolved["n_factors"] = n_factors

#     nutrients = process.nutrients.resolve(ctx)
#     return_rate = process.return_rate.resolve(ctx)
#     conditions = [max(0.0, min(1.0, float(x))) for x in process.conditions.resolve(ctx)]
#     limiting_factors = process.limiting_factors.resolve(ctx)
#     prospecting = process.prospecting_scan_sharpness.resolve(ctx)

#     return NicheGen(
#         surface=float(surface),
#         ecological_health=1.0,
#         nutrients=nutrients,
#         return_rate=return_rate,
#         conditions=conditions,
#         limiting_factors=limiting_factors,
#         prospecting_scan_sharpness=prospecting,
#         populations=[],
#     )

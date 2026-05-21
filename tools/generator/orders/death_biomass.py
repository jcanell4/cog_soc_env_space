# """Composite orders for ``generation_config.death_biomass``."""

# from __future__ import annotations

# from dataclasses import dataclass, field
# from typing import Any, Dict, Mapping, Union

# from .base import DefaultOrder, Order
# from .context import GenerationContext
# from .defaults import FieldDefaultCatalog
# from .nested import FromContext, Partial, StructOrder
# from .scalar import Literal
# from .util import coerce_int_keys


# def _resolve_subfield(val: Any, ctx: GenerationContext) -> Any:
#     if val is None:
#         return None
#     if isinstance(val, DefaultOrder):
#         val = val.resolve(ctx)
#     if isinstance(val, Order):
#         return val.resolve(ctx)
#     return val


# @dataclass(frozen=True)
# class BinSizeMap(Order[Dict[int, int]]):
#     """Bin index -> plant size tier (partial dict merged with defaults)."""

#     mapping: Union[Partial[Dict[int, int]], Dict[int, int], DefaultOrder]

#     def resolve(self, ctx: GenerationContext) -> Dict[int, int]:
#         raw = _resolve_subfield(self.mapping, ctx)
#         return {int(k): int(v) for k, v in coerce_int_keys(raw).items()}


# @dataclass(frozen=True)
# class Proportion(Order[Dict[int, Dict[int, float]]]):
#     """Stage -> bin -> fixed fraction (partial); free bins filled at build time."""

#     mapping: Union[Partial[Dict[int, Dict[int, float]]], Dict[Any, Any], DefaultOrder]

#     def resolve(self, ctx: GenerationContext) -> Dict[int, Dict[int, float]]:
#         raw = _resolve_subfield(self.mapping, ctx)
#         out: Dict[int, Dict[int, float]] = {}
#         for sk, inner in coerce_int_keys(raw).items():
#             if not isinstance(inner, dict):
#                 raise ValueError(f"Proportion: stage {sk} must map bin -> float")
#             out[int(sk)] = {int(bk): float(bv) for bk, bv in coerce_int_keys(inner).items()}
#             total = sum(out[int(sk)].values())
#             if total > 1.0 + 1e-9:
#                 raise ValueError(f"Proportion: fixed fractions for stage {sk} sum to {total} > 1")
#         return out


# class DeathBiomassDefaultCatalog:
#     bins = FieldDefaultCatalog(
#         {"from_context": FromContext("n_bins"), "three": Literal(3.0)},
#         active="from_context",
#     )
#     size = FieldDefaultCatalog(
#         {"empty": BinSizeMap(Partial({}))},
#         active="empty",
#     )
#     fraction = FieldDefaultCatalog(
#         {"empty": Proportion(Partial({}))},
#         active="empty",
#     )

#     @classmethod
#     def field_catalogs(cls) -> Mapping[str, FieldDefaultCatalog]:
#         return {
#             "bins": cls.bins,
#             "size": cls.size,
#             "fraction": cls.fraction,
#         }


# @dataclass
# class DeathBiomassOrder(StructOrder):
#     bins: Union[Order, float, int] = field(default_factory=DefaultOrder)
#     size: Union[BinSizeMap, Order, Dict[int, int], DefaultOrder] = field(default_factory=DefaultOrder)
#     fraction: Union[Proportion, Order, Dict[Any, Any], DefaultOrder] = field(
#         default_factory=DefaultOrder
#     )
#     characteristics: Union[Order, list, tuple, None] = None

#     def subfield_catalogs(self) -> Mapping[str, FieldDefaultCatalog]:
#         return DeathBiomassDefaultCatalog.field_catalogs()

#     def resolve(self, ctx: GenerationContext) -> Dict[str, Any]:
#         if not self._defaults_applied:
#             self.apply_subdefaults()

#         out: Dict[str, Any] = {}

#         bins_val = int(_resolve_subfield(self.bins, ctx))
#         if bins_val < 2:
#             raise ValueError("death_biomass.bins must be >= 2")
#         out["bins"] = bins_val

#         if isinstance(self.size, BinSizeMap):
#             out["size"] = self.size.resolve(ctx)
#         else:
#             size_raw = _resolve_subfield(self.size, ctx)
#             out["size"] = {int(k): int(v) for k, v in coerce_int_keys(size_raw or {}).items()}

#         if isinstance(self.fraction, Proportion):
#             out["fraction"] = self.fraction.resolve(ctx)
#         else:
#             frac_raw = _resolve_subfield(self.fraction, ctx)
#             out["fraction"] = Proportion(frac_raw or {}).resolve(ctx)

#         if self.characteristics is not None:
#             ch = _resolve_subfield(self.characteristics, ctx)
#             if isinstance(ch, (list, tuple)) and len(ch) == 2:
#                 lo, hi = float(ch[0]), float(ch[1])
#                 if lo > hi:
#                     lo, hi = hi, lo
#                 out["characteristics"] = [lo, hi]

#         return out

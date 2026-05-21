# """Composite orders used by species profiles."""

# from __future__ import annotations

# from dataclasses import dataclass
# from typing import Any, Dict, List, Optional, Sequence

# from .base import Order
# from .context import GenerationContext
# from .scalar import Gaussian, Literal
# from .vector import LiteralVector


# @dataclass(frozen=True)
# class EnergyFactor(Order[Dict[str, Any]]):
#     """Resolve to ``generation_config`` energy entry (number = std only, or mean/std object)."""

#     mean: Optional[float] = None
#     std: Optional[float] = None

#     def resolve(self, ctx: GenerationContext) -> Dict[str, Any]:
#         if self.mean is not None and self.std is not None:
#             if ctx.stochastic:
#                 return {"mean": self.mean, "std": self.std}
#             return {"mean": self.mean, "std": self.std}
#         if self.std is not None:
#             return {"std": float(self.std)}
#         if self.mean is not None:
#             return {"mean": float(self.mean)}
#         return {}


# @dataclass(frozen=True)
# class LiteralDict(Order[Dict[str, Any]]):
#     value: Dict[str, Any]

#     def resolve(self, ctx: GenerationContext) -> Dict[str, Any]:
#         return dict(self.value)


# @dataclass(frozen=True)
# class SizeClasses(Order[List[int]]):
#     classes: Sequence[int]

#     def resolve(self, ctx: GenerationContext) -> List[int]:
#         return [int(x) for x in self.classes]


# @dataclass(frozen=True)
# class SizeClassDict(Order[Dict[str, List[int]]]):
#     """Heterotroph ``size`` block: ``biomass_classes`` / ``surface_classes`` per stage."""

#     classes: Sequence[int]

#     def resolve(self, ctx: GenerationContext) -> Dict[str, List[int]]:
#         tiers = [int(x) for x in self.classes]
#         return {"biomass_classes": tiers, "surface_classes": list(tiers)}


# @dataclass(frozen=True)
# class LiteralValue(Order[Any]):
#     """Pass any JSON-serializable literal (e.g. ``diet_by_food_type`` matrix)."""

#     value: Any

#     def resolve(self, ctx: GenerationContext) -> Any:
#         return self.value

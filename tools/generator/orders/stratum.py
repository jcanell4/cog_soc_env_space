# """Composite orders for ``generation_config.stratum_config``."""

# from __future__ import annotations

# from dataclasses import dataclass, field
# from typing import Any, Dict, List, Mapping, Sequence, Union

# from .base import DefaultOrder, Order
# from .context import GenerationContext
# from .defaults import FieldDefaultCatalog
# from .nested import Partial, StructOrder
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
# class DefaultStrataCount(Order[int]):
#     """``max(3, n_stages + 2)`` — matches legacy default when stratum_config omitted."""

#     def resolve(self, ctx: GenerationContext) -> int:
#         n = max(1, int(ctx.n_stages))
#         return max(3, min(8, n + 2))


# @dataclass(frozen=True)
# class SlipLastValue(Order[Any]):
#     """
#     Extend a sequence or stage->height map to ``n_stages`` by repeating the last value.

#     Wraps an inner order or literal partial content.
#     """

#     inner: Union[Partial[Any], Order, Dict[int, int], List[int], Sequence[int]]

#     def resolve(self, ctx: GenerationContext) -> List[int]:
#         raw = _resolve_subfield(self.inner, ctx)
#         n = max(1, int(ctx.n_stages))

#         if isinstance(raw, dict):
#             m = {int(k): int(v) for k, v in coerce_int_keys(raw).items()}
#             out: List[int] = []
#             last = m.get(0, 0)
#             for s in range(n):
#                 if s in m:
#                     last = m[s]
#                 out.append(max(0, min(4, last)))
#             return out

#         if isinstance(raw, (list, tuple)):
#             seq = [max(0, min(4, int(x))) for x in raw]
#             if not seq:
#                 return [0] * n
#             if len(seq) >= n:
#                 return seq[:n]
#             last = seq[-1]
#             return seq + [last] * (n - len(seq))

#         return [0] * n


# @dataclass(frozen=True)
# class StageHeightMap(Order[Any]):
#     """Stage height classes as dict or list; optional SlipLastValue / Partial wrapper."""

#     mapping: Union[SlipLastValue, Partial[Any], Order, Dict[int, int], List[int], Sequence[int]]

#     def resolve(self, ctx: GenerationContext) -> Union[Dict[int, int], List[int]]:
#         if isinstance(self.mapping, SlipLastValue):
#             return self.mapping.resolve(ctx)
#         raw = _resolve_subfield(self.mapping, ctx)
#         n = max(1, int(ctx.n_stages))
#         if isinstance(raw, dict):
#             return {int(k): max(0, min(4, int(v))) for k, v in coerce_int_keys(raw).items()}
#         if isinstance(raw, (list, tuple)):
#             seq = [max(0, min(4, int(x))) for x in raw]
#             if len(seq) >= n:
#                 return seq[:n]
#             if not seq:
#                 return [0] * n
#             return seq + [seq[-1]] * (n - len(seq))
#         return [0] * n


# class StratumConfigDefaultCatalog:
#     n_strata = FieldDefaultCatalog(
#         {"auto": DefaultStrataCount(), "two": Literal(2.0), "three": Literal(3.0)},
#         active="auto",
#     )
#     stage_height_class = FieldDefaultCatalog(
#         {
#             "slip_zeros": StageHeightMap(SlipLastValue(Partial({0: 0}))),
#             "profile_001": StageHeightMap(SlipLastValue(Partial({0: 0, 1: 0, 2: 1}))),
#         },
#         active="profile_001",
#     )

#     @classmethod
#     def field_catalogs(cls) -> Mapping[str, FieldDefaultCatalog]:
#         return {
#             "n_strata": cls.n_strata,
#             "stage_height_class": cls.stage_height_class,
#         }


# @dataclass
# class StratumConfigOrder(StructOrder):
#     n_strata: Union[Order, int, float] = field(default_factory=DefaultOrder)
#     stage_height_class: Union[StageHeightMap, SlipLastValue, Order, Dict[int, int], List[int], DefaultOrder] = (
#         field(default_factory=DefaultOrder)
#     )

#     def subfield_catalogs(self) -> Mapping[str, FieldDefaultCatalog]:
#         return StratumConfigDefaultCatalog.field_catalogs()

#     def resolve(self, ctx: GenerationContext) -> Dict[str, Any]:
#         if not self._defaults_applied:
#             self.apply_subdefaults()

#         n_strata = int(_resolve_subfield(self.n_strata, ctx))
#         if n_strata < 2:
#             raise ValueError("stratum_config.n_strata must be >= 2")

#         if isinstance(self.stage_height_class, (StageHeightMap, SlipLastValue)):
#             heights = self.stage_height_class.resolve(ctx)
#         else:
#             heights = StageHeightMap(self.stage_height_class).resolve(ctx)

#         out: Dict[str, Any] = {"n_strata": n_strata}
#         if isinstance(heights, dict):
#             out["stage_height_class"] = heights
#         else:
#             out["stage_height_class"] = list(heights)
#         return out

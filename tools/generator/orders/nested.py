# """Nested / partial orders and struct-order helpers."""

# from __future__ import annotations

# from abc import ABC, abstractmethod
# from dataclasses import dataclass, fields, is_dataclass
# from typing import Any, Dict, Generic, Mapping, Optional, TypeVar, Union

# from .base import DefaultOrder, Order
# from .context import GenerationContext
# from .defaults import FieldDefaultCatalog
# from .util import deep_merge

# T = TypeVar("T")


# @dataclass(frozen=True)
# class FromContext(Order[Any]):
#     """Read a value from :class:`GenerationContext` or ``ctx.resolved``."""

#     key: str

#     def resolve(self, ctx: GenerationContext) -> Any:
#         if self.key in ctx.resolved:
#             return ctx.resolved[self.key]
#         if hasattr(ctx, self.key):
#             return getattr(ctx, self.key)
#         raise ValueError(f"FromContext: unknown key {self.key!r}")


# @dataclass
# class Partial(Order[T], Generic[T]):
#     """
#     Partial override merged on top of a default order.

#     ``default`` may be a concrete order, ``DefaultOrder`` (needs ``set_defaults``), or None
#     (treated as empty dict / empty mapping at resolve time).
#     """

#     inner: Union[Order[T], T]
#     default: Optional[Union[Order[T], DefaultOrder[T]]] = None

#     def set_defaults(
#         self,
#         catalog: Union[FieldDefaultCatalog[T], Mapping[str, Order[T]], Any],
#         *,
#         key: str | int | None = None,
#     ) -> Partial[T]:
#         if isinstance(self.default, DefaultOrder):
#             self.default.set_defaults(catalog, key=key)
#         return self

#     def resolve(self, ctx: GenerationContext) -> T:
#         if self.default is None:
#             base: Any = {}
#         elif isinstance(self.default, Order):
#             base = self.default.resolve(ctx)
#         else:
#             base = self.default

#         if isinstance(self.inner, Order):
#             over = self.inner.resolve(ctx)
#         else:
#             over = self.inner

#         return deep_merge(base, over)  # type: ignore[return-value]


# class StructOrder(Order[Dict[str, Any]], ABC):
#     """Composite order: each dataclass field is a sub-order resolved into a dict."""

#     _defaults_applied: bool = False

#     @abstractmethod
#     def subfield_catalogs(self) -> Mapping[str, FieldDefaultCatalog]:
#         ...

#     def apply_subdefaults(self) -> None:
#         catalogs = self.subfield_catalogs()
#         if not is_dataclass(self):
#             raise TypeError("StructOrder implementors must be dataclasses")
#         for f in fields(self):
#             if f.name.startswith("_"):
#                 continue
#             val = getattr(self, f.name)
#             if isinstance(val, DefaultOrder) and f.name in catalogs:
#                 val.set_defaults(catalogs[f.name])
#             elif isinstance(val, Partial) and val.default is not None:
#                 if isinstance(val.default, DefaultOrder) and f.name in catalogs:
#                     val.default.set_defaults(catalogs[f.name])
#         self._defaults_applied = True

#     def resolve(self, ctx: GenerationContext) -> Dict[str, Any]:
#         if not self._defaults_applied:
#             self.apply_subdefaults()
#         out: Dict[str, Any] = {}
#         if not is_dataclass(self):
#             raise TypeError("StructOrder implementors must be dataclasses")
#         for f in fields(self):
#             if f.name.startswith("_"):
#                 continue
#             val = getattr(self, f.name)
#             if val is None:
#                 continue
#             if isinstance(val, Order):
#                 resolved = val.resolve(ctx)
#             else:
#                 resolved = val
#             if resolved is not None and resolved != {}:
#                 out[f.name] = resolved
#         return out


# def apply_subdefaults(struct: StructOrder) -> None:
#     struct.apply_subdefaults()

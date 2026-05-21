# """Named / indexed default catalogs for generation fields."""

# from __future__ import annotations

# from typing import TYPE_CHECKING, Generic, List, Mapping, Sequence, TypeVar, Union

# if TYPE_CHECKING:
#     from .base import Order

# T = TypeVar("T")


# class FieldDefaultCatalog(Generic[T]):
#     """Several candidate orders per field; pick by name or index."""

#     def __init__(
#         self,
#         variants: Union[Mapping[str, "Order[T]"], Sequence["Order[T]"]],
#         *,
#         active: str | int | None = None,
#     ) -> None:
#         self._variants = variants
#         self._active = active

#     def set_active(self, key: str | int) -> None:
#         self._active = key

#     @property
#     def active(self) -> str | int | None:
#         return self._active

#     def pick(self, key: str | int | None = None) -> "Order[T]":
#         k = key if key is not None else self._active
#         if isinstance(self._variants, Mapping):
#             if k is None:
#                 raise ValueError("default key required for mapping catalog")
#             if k not in self._variants:
#                 raise KeyError(f"unknown default key {k!r}; available: {list(self._variants)}")
#             return self._variants[k]
#         seq = self._variants
#         if k is None:
#             k = 0
#         idx = int(k)
#         if idx < 0 or idx >= len(seq):
#             raise IndexError(f"default index {idx} out of range for catalog of length {len(seq)}")
#         return seq[idx]

#     def keys(self) -> List[str]:
#         if isinstance(self._variants, Mapping):
#             return list(self._variants.keys())
#         return [str(i) for i in range(len(self._variants))]

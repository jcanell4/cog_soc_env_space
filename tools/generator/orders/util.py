# """Shared helpers for order resolution."""

# from __future__ import annotations

# from typing import Any, Dict


# def coerce_int_keys(m: Any) -> Dict[int, Any]:
#     if not isinstance(m, dict):
#         return {}
#     out: Dict[int, Any] = {}
#     for k, v in m.items():
#         try:
#             out[int(k)] = v
#         except (TypeError, ValueError) as exc:
#             raise ValueError(f"Expected integer-like dict keys, got {k!r}") from exc
#     return out


# def deep_merge(base: Any, override: Any) -> Any:
#     """Recursively merge override into base (override wins at leaves)."""
#     if override is None:
#         return base
#     if base is None:
#         return override
#     if isinstance(base, dict) and isinstance(override, dict):
#         out = dict(base)
#         for k, v in override.items():
#             if k in out and isinstance(out[k], dict) and isinstance(v, dict):
#                 out[k] = deep_merge(out[k], v)
#             else:
#                 out[k] = v
#         return out
#     return override

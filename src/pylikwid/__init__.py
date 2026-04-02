import functools

from .pylikwid import *


def profile(_func=None, *, region_name=None):
    """Decorator that wraps a function in a LIKWID marker region.

    Usage::

        @profile
        def my_func(): ...          # region name = "my_func"

        @profile(region_name="work")
        def my_func(): ...          # region name = "work"
    """
    def decorator(func):
        name = region_name if region_name is not None else func.__name__

        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            markerstartregion(name)
            try:
                return func(*args, **kwargs)
            finally:
                markerstopregion(name)

        return wrapper

    if _func is not None:
        # Used as @profile without parentheses
        return decorator(_func)
    # Used as @profile(...) with parentheses
    return decorator

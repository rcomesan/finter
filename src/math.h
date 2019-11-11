#ifndef MATH_H_
#define MATH_H_

#include <cmath>

namespace finter
{
    inline float ffloor(float _f)
    {
        return std::floorf(_f);
    }

    inline float fceil(float _f)
    {
        return std::ceilf(_f);
    }

    inline float fround(float _f)
    {
        return ffloor(_f + 0.5f);
    }

    inline float fmin(float _a, float _b)
    {
        return _a < _b ? _a : _b;
    }

    inline float fmax(float _a, float _b)
    {
        return _a > _b ? _a : _b;
    }

    inline float i32min(int32_t _a, int32_t _b)
    {
        return _a < _b ? _a : _b;
    }

    inline float i32max(int32_t _a, int32_t _b)
    {
        return _a > _b ? _a : _b;
    }

    inline float fclamp(float _a, float _min, float _max)
    {
        return fmin(fmax(_a, _min), _max);
    }

    inline float fmap(float _v, float _fromMin, float _fromMax, float _toMin, float _toMax, bool _clampResult = true)
    {
        _v = (_v - _fromMin) / (_fromMax - _fromMin) * (_toMax - _toMin) + _toMin;

        return _clampResult
            ? fclamp(_v, _toMin, _toMax)
            : _v;
    }
}
#endif // MATH_H_
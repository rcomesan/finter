#include "interpolator.h"
#include "math.h"

#include <limits.h>
#include <string.h>
#include <inttypes.h>

namespace finter
{
    Interpolation::Interpolation()
    {
        ZERO_MEM(name);
    }

    Interpolation::~Interpolation()
    {
    }

    bool Interpolation::parseData(const char * _inBuff, std::vector<ImVec2>& _outData)
    {
        const char* p = _inBuff;
        char        buff[25];
        uint32_t    buffPos = 0;
        ImVec2      point;

        while (*p)
        {
            if (*p == ',' || *p == ';')
            {
                buff[buffPos] = '\0';
                buffPos = 0;
            }

            if (*p == ',')
            {
                point.x = strtof(buff, NULL);
            }
            else if (*p == ';')
            {
                // end of ordered pair
                point.y = strtof(buff, NULL);
                _outData.push_back(point);
            }
            else
            {
                buff[buffPos++] = *p;
            }

            p++;
        }

        if (buffPos != 0)
        {
            buff[buffPos] = '\0';
            point.y = strtof(buff, NULL);
            _outData.push_back(point);
        }

        return true;
    }

    void Interpolation::recalculateDiffs()
    {
        Newton::calculateDiffs(datapoints, diffs);
    }

    float Lagrange::lx(std::vector<ImVec2>& _dp, uint32_t _i, float _x)
    {
        float dividend = 1;
        float divisor = 1;

        for (uint32_t index = 0; index < _dp.size(); index++)
        {
            if (index != _i)
            {
                dividend *= (_x - _dp[index].x);
                divisor *= (_dp[_i].x - _dp[index].x);
            }
        }
        return dividend / divisor;
    }

    float Lagrange::eval(std::vector<ImVec2>& _dp, float _x)
    {
        if (0 == _dp.size()) return 0.0f;

        float r = 0.0f;

        for (uint32_t i = 0; i < _dp.size(); i++)
        {
            r += _dp[i].y * lx(_dp, i, _x);
        }

        return r;
    }

    void Lagrange::latexPx(std::vector<ImVec2>& _dp, std::string& _out)
    {
        static char buff[255];
        static float v;
        static char  s;

        _out = "P(x) = ";

        for (uint32_t i = 0; i < _dp.size(); i++)
        {
            simplifySigns(true, _dp[i].y, s, v);
            if (i == 0 && s == '+') s = ' ';

            snprintf(buff, sizeof(buff), "%c %.4g \\cdot L_{%" PRIu32 "}(x)", s, v, i);
            _out.append(buff);
        }
    }

    void Lagrange::latexLx(std::vector<ImVec2>& _dp, uint32_t _i, std::string& _out)
    {
        static char buff[255];
        static float v;
        static char  s;

        static std::string tmp;
        tmp.reserve(2500);
        tmp = "{";

        snprintf(buff, sizeof(buff), "L_{%" PRIu32 "}(x) = ", _i);
        _out = std::string(buff);
        _out.reserve(2500);
        _out.append("{{");

        for (uint32_t index = 0; index < _dp.size(); index++)
        {
            if (index != _i)
            {
                simplifySigns(false, _dp[index].x, s, v);
                snprintf(buff, sizeof(buff), "(x  %c %.4g)", s, v);
                _out.append(buff);

                simplifySigns(false, _dp[index].x, s, v);
                snprintf(buff, sizeof(buff), "(%.4g %c %.4g)", _dp[_i].x, s, v);
                tmp.append(buff);
            }
        }

        tmp.append("}");
        _out.append("} \\above{1pt} ");
        _out.append(tmp);
        _out.append("}");
    }

    float Newton::eval(std::vector<ImVec2>& _dp, float _x, bool _pro, std::vector<std::vector<float>>& _diffs)
    {
        if (0 == _dp.size()) return 0.0f;

        float r = _dp[_pro ? 0 : _dp.size() - 1].y;
        float prod;

        for (uint32_t i = 1; i < _diffs.size(); i++)
        {
            prod = 1.0f;
            for (uint32_t j = 0; j <= i - 1; j++)
            {
                prod *= (_x - _dp[_pro ? j : _dp.size() - 1 - j].x);
            }

            r += (_diffs[i][_pro ? 0 : _diffs[i].size() - 1] * prod);
        }

        return r;
    }

    float Newton::eval(std::vector<ImVec2>& _dp, float _x, bool _pro)
    {
        std::vector<std::vector<float>> diffs;
        Newton::calculateDiffs(_dp, diffs);

        return Newton::eval(_dp, _x, _pro, diffs);
    }

    void Newton::latexFormula(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, bool _pro, std::string& _out)
    {
        static char buff[255];

        _out.reserve(2500);

        snprintf(buff, sizeof(buff), "P(x)=f_%" PRIu32, _pro ? 0 : _dp.size() - 1);
        _out = std::string(buff);

        for (uint32_t i = 1; i < _diffs.size(); i++)
        {
            _out.append(" + f[x_0");
            for (uint32_t j = 0; j < i; j++)
            {
                snprintf(buff, sizeof(buff), "; x_%" PRIu32 "", j + 1);
                _out.append(buff);
            }
            _out.append("]");

            for (uint32_t j = 0; j <= i - 1; j++)
            {
                snprintf(buff, sizeof(buff), " (x - x_%" PRIu32 ")", _pro ? j : _dp.size() - 1 - j);
                _out.append(buff);
            }
        }
    }

    void Newton::latexPx(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, bool _pro, std::string& _out)
    {
        static char  buff[255];
        static float v;
        static char  s;

        snprintf(buff, sizeof(buff), "P(x)=%.4g", _pro ? _dp[0].y : _dp[_dp.size() - 1].y);

        _out.reserve(2500);
        _out = std::string(buff);

        for (uint32_t i = 1; i < _diffs.size(); i++)
        {
            simplifySigns(true, _diffs[i][_pro ? 0 : _diffs[i].size() - 1], s, v);
            snprintf(buff, sizeof(buff), " %c %.4g \\cdot", s, v);
            _out.append(buff);


            for (uint32_t j = 0; j <= i - 1; j++)
            {
                simplifySigns(false, _dp[_pro ? j : _dp.size() - 1 - j].x, s, v);
                snprintf(buff, sizeof(buff), " (x %c %.4g)", s, v);
                _out.append(buff);
            }
        }
    }

    void Newton::latexFx(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, bool _pro, uint32_t _from, uint32_t _to, std::string& _out)
    {
        static char buff[255];

        _out.reserve(2500);

        snprintf(buff, sizeof(buff), "f[x_%" PRIu32 "", _from);
        _out = std::string(buff);

        for (uint32_t i = _from + 1; i <= _to; i++)
        {
            snprintf(buff, sizeof(buff), ";x_%" PRIu32 "", i);
            _out.append(buff);
        }
        _out.append("] = ");

        uint32_t diffOrder = _to - _from;
        uint32_t diffOrderPrev = diffOrder - 1;

        float v1, v2;
        char s1, s2;

        simplifySigns(false, Newton::getY(_dp, _diffs, diffOrderPrev, _from), s1, v1);
        simplifySigns(false, _dp[_from].x, s2, v2);

        snprintf(buff, sizeof(buff), "{{%.4g %c %.4g} \\above{1pt} {%.4g %c %.4g}} = %.4g",
            Newton::getY(_dp, _diffs, diffOrderPrev, _from + 1), s1, v1,
            _dp[_to].x, s2, v2,
            _diffs[diffOrder][_from]);
        _out.append(buff);
    }

    void Newton::calculateDiffs(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _outDiffs)
    {
        _outDiffs.resize(_dp.size());

        uint32_t order = 1;
        uint32_t prevOrderSize = _dp.size();

        while (prevOrderSize > 1)
        {
            std::vector<float>& d = _outDiffs[order];

            // calculate f[x[i], ..., x(i+n)]
            d.resize(prevOrderSize - 1);
            for (uint32_t i = 0; i < prevOrderSize - 1; i++)
            {
                d[i] = (Newton::getY(_dp, _outDiffs, order - 1, i + 1) - Newton::getY(_dp, _outDiffs, order - 1, i))
                    / (_dp[i + order].x - _dp[i].x);
            }

            prevOrderSize = d.size();
            order++;
        }

        _outDiffs.resize(order - 1); // we get rid of the last difference since it will always be zero.
    }

    float Newton::getY(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, uint32_t _order, uint32_t _index)
    {
        if (_order < 1)
        {
            return _dp[_index].y;
        }
        else
        {
            return _diffs[_order][_index];
        }
    }

    Interpolator::Interpolator()
    {
        container.reserve(100);
    }

    Interpolator::~Interpolator()
    {
        container.clear();
    }

    void Interpolator::add(Interpolation& _interpolation)
    {
        container.push_back(_interpolation); // <-- this involves a copy

        Interpolation& newIntp = container.back();
        newIntp.recalculateDiffs();

        if (0 == strlen(newIntp.name))
            snprintf(newIntp.name, INTERPOLATION_NAME_LEN, "interpolation at 0x%p", (void*)&newIntp);
    }

    void Interpolator::removeAt(uint32_t _index)
    {
        container.erase(container.begin() + _index);
    }

    Interpolation& Interpolator::getAt(uint32_t _index)
    {
        return container[_index];
    }

    Interpolation* Interpolator::getInterpolations()
    {
        return (container.size() > 0) 
            ? &container[0]
            : nullptr;
    }

    uint32_t Interpolator::getCount()
    {
        return container.size();
    }
}
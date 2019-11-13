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

        _out = "P(x) = ";

        for (uint32_t i = 0; i < _dp.size(); i++)
        {
            snprintf(buff, sizeof(buff), "%s %.4g \\cdot L_{%" PRIu32 "}(x)", 
                _dp[i].y > 0 && i != 0 ? "+" : "",
                _dp[i].y, i);
            _out.append(buff);
        }
    }

    void Lagrange::latexLx(std::vector<ImVec2>& _dp, uint32_t _i, std::string& _out)
    {
        static char buff[255];

        static std::string tmp;
        tmp.reserve(5000);
        tmp = "{";

        snprintf(buff, sizeof(buff), "L_{%" PRIu32 "}(x) = ", _i);
        _out = std::string(buff);
        _out.reserve(5000);
        _out.append("{{");

        for (uint32_t index = 0; index < _dp.size(); index++)
        {
            if (index != _i)
            {
                snprintf(buff, sizeof(buff), "(x  %c %.4g)", 
                    _dp[index].x > 0 ? '-' : '+', 
                    (_dp[index].x > 0 ? 1 : -1) * _dp[index].x);
                _out.append(buff);

                snprintf(buff, sizeof(buff), "(%.4g %c %.4g)", 
                    _dp[_i].x, _dp[index].x > 0 ? '-' : '+', 
                    (_dp[index].x > 0 ? 1 : -1) * _dp[index].x);
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
    
    static float eval(std::vector<ImVec2>& _dp, float _x, bool _pro)
    {
        std::vector<std::vector<float>> diffs;
        Newton::calculateDiffs(_dp, diffs);

        return Newton::eval(_dp, _x, _pro, diffs);
    }

    void Newton::calculateDiffs(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _outDiffs)
    {
        _outDiffs.resize(_dp.size());

        static std::vector<float>  datapointsYs;
        datapointsYs.resize(_dp.size());
        for (uint32_t i = 0; i < _dp.size(); i++)
        {
            datapointsYs[i] = _dp[i].y;
        }

        std::vector<float>& prevOrderYs = datapointsYs;
        uint32_t n = 1;

        while (prevOrderYs.size() > 1)
        {
            // order "n" differences
            std::vector<float>& d = _outDiffs[n];

            // calculate f[x[i], ..., x(i+n)]
            d.resize(prevOrderYs.size() - 1);
            for (uint32_t i = 0; i < prevOrderYs.size() - 1; i++)
            {
                d[i] = (prevOrderYs[i + 1] - prevOrderYs[i]) / (_dp[i + n].x - _dp[i].x);
            }

            prevOrderYs = d;
            n++;
        }

        _outDiffs.resize(n - 1); // we get rid of the last difference since it will always be zero.
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
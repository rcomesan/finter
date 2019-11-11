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

    void Interpolation::recalculate()
    {
        recalculateLagrange();
        recalculateNewton(true);
        recalculateNewton(false);
    }

    void Interpolation::recalculateLagrange()
    {
    }

    void Interpolation::recalculateNewton(bool _progressive)
    {
        //TODO
    }

    float Lagrange::lx(std::vector<ImVec2>& _d, uint32_t _i, float _x)
    {
        float dividend = 1;
        float divisor = 1;

        for (uint32_t index = 0; index < _d.size(); index++)
        {
            if (index != _i)
            {
                dividend *= (_x - _d[index].x);
                divisor *= (_d[_i].x - _d[index].x);
            }
        }
        return dividend / divisor;
    }

    float Lagrange::eval(std::vector<ImVec2>& _d, float _x)
    {
        if (0 == _d.size()) return 0.0f;

        float r = 0.0f;

        for (uint32_t i = 0; i < _d.size(); i++)
        {
            r += _d[i].y * lx(_d, i, _x);
        }

        return r;
    }

    void Lagrange::latexPx(std::vector<ImVec2>& _d, std::string& _out)
    {
        static char buff[255];

        _out = "P(x) = ";

        for (uint32_t i = 0; i < _d.size(); i++)
        {
            snprintf(buff, sizeof(buff), "%s %.4g \\cdot L_{%" PRIu32 "}(x)", 
                _d[i].y > 0 && i != 0 ? "+" : "",
                _d[i].y, i);
            _out.append(buff);
        }
    }

    void Lagrange::latexLx(std::vector<ImVec2>& _d, uint32_t _i, std::string& _out)
    {
        static char buff[255];

        static std::string tmp;
        tmp.reserve(5000);
        tmp = "{";

        snprintf(buff, sizeof(buff), "L_{%" PRIu32 "}(x) = ", _i);
        _out = std::string(buff);
        _out.reserve(5000);
        _out.append("{{");

        for (uint32_t index = 0; index < _d.size(); index++)
        {
            if (index != _i)
            {
                snprintf(buff, sizeof(buff), "(x  %c %.4g)", 
                    _d[index].x > 0 ? '-' : '+', 
                    (_d[index].x > 0 ? 1 : -1) * _d[index].x);
                _out.append(buff);

                snprintf(buff, sizeof(buff), "(%.4g %c %.4g)", 
                    _d[_i].x, _d[index].x > 0 ? '-' : '+', 
                    (_d[index].x > 0 ? 1 : -1) * _d[index].x);
                tmp.append(buff);
            }
        }

        tmp.append("}");
        _out.append("} \\above{1pt} ");
        _out.append(tmp);
        _out.append("}");
    }

    float Newton::eval(std::vector<ImVec2>& _d, float _x)
    {
        if (0 == _d.size()) return 0.0f;
        //TODO
        return 0.0f;
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
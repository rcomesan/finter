#include "interpolator.h"
#include "math.h"

#include <limits.h>
#include <string.h>

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
        //TODO
    }

    void Interpolation::recalculateNewton(bool _progressive)
    {
        //TODO
    }

    float Lagrange::l(std::vector<ImVec2>& _d, uint32_t _i, float _x)
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
            r += _d[i].y * l(_d, i, _x);
        }

        return r;
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
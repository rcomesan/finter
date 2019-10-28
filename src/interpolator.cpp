#include "interpolator.h"

#include <string.h>

namespace finter
{
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
        container.push_back(_interpolation);

        if (0 == strlen(container.back().name))
            snprintf(container.back().name, INTERPOLATION_NAME_LEN, "interpolation at 0x%p", (void*)&container.back());
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
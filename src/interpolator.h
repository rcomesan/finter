#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_

#include "defines.h"
#include "imgui.h"

#include <string>
#include <vector>
#include <stdint.h>

namespace finter
{

    struct Interpolation
    {
        char                        name[INTERPOLATION_NAME_LEN];
        std::vector<ImVec2>         datapoints;

        static bool parseData(const char* _inBuff, std::vector<ImVec2>& _outData);
    };

    class Interpolator
    {
    public:
                                    Interpolator();
                                    ~Interpolator();

        void                        add(Interpolation& _interpolation);
        void                        removeAt(uint32_t _index);
        Interpolation&              getAt(uint32_t _index);
        Interpolation*              getInterpolations();
        uint32_t                    getCount();

    private:
        std::vector<Interpolation>  container;
    };
}

#endif // INTERPOLATOR_H_


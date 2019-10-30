#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_

#include "defines.h"
#include "imgui.h"

#include <string>
#include <vector>
#include <stdint.h>

namespace finter
{
    /*
    struct Polynomial
    {
        std::vector<float> coeffs;
        
        Polynomial()
        {
            coeffs.reserve(100);
        }

        inline uint32_t degree()
        {
            return coeffs.size() - 1;
        }

        inline float eval(float _x)
        {
            float im;
            for (uint32_t i = 0; i < coeffs.size(); i++)
            {
                im += coeffs[i] * powf(_x, i);
            }
            return im;
        }
    };
    */

    struct Lagrange
    {
        static float eval(std::vector<ImVec2>& _d, float _x);

    private:
        static float l(std::vector<ImVec2>& _d, uint32_t _i, float _x);
    };

    struct Newton
    {
        static float eval(std::vector<ImVec2>& _d, float _x);
    };

    struct Interpolation
    {
        Interpolation();
        ~Interpolation();

        /*
        enum Type
        {
            None = 0,
            Lagrange,
            NewtonProgressive,
            NewtonRegressive,
            Count
        };

        Polynomial                  p[Interpolation::Type::Count];
        */

        char                        name[INTERPOLATION_NAME_LEN];
        std::vector<ImVec2>         datapoints;
        ImVec2*                     datapointSelected;
        
        static bool                 parseData(const char* _inBuff, std::vector<ImVec2>& _outData);

        inline float                evalLagrange(float _x) { return Lagrange::eval(datapoints, _x); }
        inline float                evalNewtonPr(float _x) { return Newton::eval(datapoints, _x); }
        inline float                evalNewtonRe(float _x) { return Newton::eval(datapoints, _x); }

        void                        recalculate();

    private:    
        void                        recalculateLagrange();
        void                        recalculateNewton(bool _progressive);
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


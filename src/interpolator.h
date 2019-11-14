#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_

#include "defines.h"
#include "imgui.h"

#include <string>
#include <vector>
#include <stdint.h>

namespace finter
{
    enum InterpolationVariant
    {
        Interpolation_None,
        Interpolation_Lagrange,
        Interpolation_NewtonPr,
        Interpolation_NewtonRe,
    };

    struct Lagrange
    {
        static float eval(std::vector<ImVec2>& _dp, float _x);
        static void  latexPx(std::vector<ImVec2>& _dp, std::string& _out);
        static void  latexLx(std::vector<ImVec2>& _dp, uint32_t _i, std::string& _out);

    private:
        static float lx(std::vector<ImVec2>& _dp, uint32_t _i, float _x);
    };

    struct Newton
    {
        static float eval(std::vector<ImVec2>& _dp, float _x, bool _pro, std::vector<std::vector<float>>& _diffs);
        static float eval(std::vector<ImVec2>& _dp, float _x, bool _pro);
        static void  latexFormula(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, bool _pro, std::string& _out);
        static void  latexPx(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, bool _pro, std::string& _out);
        static void  latexFx(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, bool _pro, uint32_t _from, uint32_t _to, std::string& _out);
        static void  calculateDiffs(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _outDiffs);

    private:
        inline static float getY(std::vector<ImVec2>& _dp, std::vector<std::vector<float>>& _diffs, uint32_t _order, uint32_t _index);
    };

    struct Interpolation
    {
        Interpolation();
        ~Interpolation();

        char                            name[INTERPOLATION_NAME_LEN];
        std::vector<ImVec2>             datapoints;
        ImVec2*                         datapointSelected;
        std::vector<std::vector<float>> diffs;

        static bool                     parseData(const char* _inBuff, std::vector<ImVec2>& _outData);

        inline float                    evalLagrange(float _x) { return Lagrange::eval(datapoints, _x); }
        inline float                    evalNewtonPr(float _x) { return Newton::eval(datapoints, _x, true, diffs); }
        inline float                    evalNewtonRe(float _x) { return Newton::eval(datapoints, _x, false, diffs); }

        void                            recalculateDiffs();        
    };

    class Interpolator
    {
    public:
                                        Interpolator();
                                        ~Interpolator();

        void                            add(Interpolation& _interpolation);
        void                            removeAt(uint32_t _index);
        Interpolation&                  getAt(uint32_t _index);
        Interpolation*                  getInterpolations();
        uint32_t                        getCount();

    private:
        std::vector<Interpolation>      container;
    };
}

#endif // INTERPOLATOR_H_


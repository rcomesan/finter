#ifndef RENDERER_H_
#define RENDERER_H_

#include "interpolator.h"
#include "defines.h"
#include "math.h"
#include "imgui.h"
#include "latex.hpp"

#include <map>
#include <list>
#include <d3d11.h>

namespace finter
{
    struct GraphOption
    {
        bool                        visible;
        ImVec4                      color;
    };

    struct GraphData
    {
        std::vector<float>          yS;
        float                       min;
        float                       max;
    };

    struct LatexData
    {
        std::string                 px;
        std::vector<std::string>    steps;
    };
    
    struct TextureData
    {
        std::string                         latex;
        ID3D11ShaderResourceView*           srv;
        int32_t                             w;
        int32_t                             h;
        std::list<TextureData*>::iterator   lruIt;
    };

    class Renderer
    {
    public:
        Renderer(ID3D11Device* _pd3dDevice);
        ~Renderer();
        void Draw();

    private:
        // internal state
        ID3D11Device*               device;                     // D3D11 device pointer.

        Latex                       latex;                      // latex context instance.

        Interpolator                interpolator;               // interpolations manager.
        Interpolation*              curIntp;                    // current interpolation selected.
        ImVec2                      curPoint;                   // current point being evaluated (in plane space).
        InterpolationVariant        curVariant;                 // current interpolation variant tab active/selected.

        ImVec2                      graphPos;                   // position of the graph being rendered.
        ImVec2                      graphSize;                  // size of the graph being rendered.

        ImVec2                      rangeMin;
        ImVec2                      rangeMax;

        GraphData                   gdataLagrange;              // cached graph data for drawing lagrange poylnomial in the graph.
        GraphData                   gdataNewtonFwd;             // cached graph data for drawing newton forward poylnomial.
        GraphData                   gdataNewtonBwd;             // cached graph data for drawing newton backward polynomial .

        GraphOption                 goptLagrange;               // graph options for drawing lagrange polynomial curve.
        GraphOption                 goptNewtonFwd;              // graph options for drawing newton forward polynomial curve.
        GraphOption                 goptNewtonBwd;              // graph options for drawing newton backward polynomial curve.
        GraphOption                 goptAxes;                   // graph options for drawing graph axes.
        GraphOption                 goptDatapoints;             // graph options for drawing initial data points.
        GraphOption                 goptCurPoint;               // graph options for drawing the current point being evaluated on the left panel.

        LatexData                   latexLagrange;              // latex data for drawing math formulas for lagrange polynomial.
        LatexData                   latexNewtonFwd;             // latex data for drawing math formulas for newton forward polynomial.
        LatexData                   latexNewtonBwd;             // latex data for drawing math formulas for newton backward polynomial.

        ImVec2                      mousePointLagrange;         // current point being hovered (in plane space).
        ImVec2                      mousePointNewtonFwd;        // current point being hovered (in plane space).
        ImVec2                      mousePointNewtonBwd;        // current point being hovered (in plane space).

        bool                        stepByStepOpened;

        bool                        newIntpOpened;
        Interpolation*              newIntp;
        char                        newIntpBuff[INPUT_BUFFER_LEN];

        std::list<TextureData*>                 texLru;
        std::map<std::string, TextureData*>     texMap;

        // methods
        void                        refreshGraphValues(uint32_t _steps = 1000);
        void                        refreshLatexFormulas(InterpolationVariant _variant, bool _steps);
        void                        resetView();

        // functions for converting from/to plane and screen coordinate spaces
        inline float                planeToScreenSpaceX(float _v) { return fmap(_v, rangeMin.x, rangeMax.x, 0, graphSize.x - 1, false) + 1; };
        inline float                planeToScreenSpaceY(float _v) { return fmap(_v, rangeMax.y, rangeMin.y, 0, graphSize.y - 1, false) + 1; };
        inline float                screenToPlaneSpaceX(float _v) { return fmap(_v, 0, graphSize.x - 1, rangeMin.x, rangeMax.x, false); };
        inline float                screenToPlaneSpaceY(float _v) { return fmap(_v, 0, graphSize.y - 1, rangeMin.y, rangeMax.y, false); };

        // functions for drawing widgets on the screen
        inline bool                 isGraphPosX(float _x) { return _x >= graphPos.x && _x < graphPos.x + graphSize.x; };
        inline bool                 isGraphPosY(float _y) { return _y >= graphPos.y && _y < graphPos.y + graphSize.y; };
        void                        drawPopupNewInterpolation();
        void                        drawPopupStepByStepSolution(const char* _name, LatexData& _data);
        void                        drawListboxDataPoints(std::vector<ImVec2>& _data, ImVec2** _pointSelected);
        bool                        drawListitemPoint(ImVec2& _p, bool* _isSelected);
        void                        drawPanelLeft();
        void                        drawPanelMiddle();
        void                        drawPanelBottom();
        bool                        drawTab(const char* _name, LatexData& _latex);
        void                        drawGraphAxes(ImDrawList* _dl, const ImVec4& _color);
        void                        drawGraphPoint(ImDrawList * _dl, ImVec2& _p, bool _isSelected, ImVec4& _color, bool _square = false, float _radius = 4.0f);
        void                        drawGraphCurve(std::vector<float>& _yValues, float _yMin, float _yMax, const ImVec4& _color, const ImVec2& _size);
        void                        drawOption(const char* _label, GraphOption& _opt);
        void                        drawLatex(const char* _latex);
        
        // imgui helpers
        static void                 pushDisabled();
        static void                 popDisabled();
        static void                 helpMarker(const char* _desc);

        bool                        loadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
    };
}

#endif // RENDERER_H_

#ifndef RENDERER_H_
#define RENDERER_H_

#include "interpolator.h"
#include "defines.h"
#include "math.h"
#include "imgui.h"

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

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();
        void Draw();

    private:
        // internal state
        Interpolator                interpolator;               // interpolations manager
        Interpolation*              curIntp;                    // current interpolation selected
        ImVec2                      curPoint;                   // current point being evaluated (in plane space)
        ImVec2                      mousePoint;                 // current point being hovered (in plane space)

        ImVec2                      graphPos;                   // position of the graph being rendered
        ImVec2                      graphSize;                  // size of the graph being rendered

        ImVec2                      rangeMin;
        ImVec2                      rangeMax;

        GraphData                   gdataLagrange;              // cached graph data for drawing lagrange poylnomial in the graph
        GraphData                   gdataNewtonPr;              // cached graph data for drawing newton progressive poylnomial
        GraphData                   gdataNewtonRe;              // cached graph data for drawing newton rggressive polynomial 

        GraphOption                 goptLagrange;               // graph options for drawing lagrange polynomial curve
        GraphOption                 goptNewtonPr;               // graph options for drawing newton progressive polynomial curve
        GraphOption                 goptNewtonRe;               // graph options for drawing newton regressive polynomial curve
        GraphOption                 goptAxes;                   // graph options for drawing graph axes
        GraphOption                 goptDatapoints;             // graph options for drawing initial data points
        GraphOption                 goptCurPoint;               // graph options for drawing the current point being evaluated on the left panel
        GraphOption                 goptMousePoint;             // graph options for drawing the current point under the mouse cursor

        bool                        newIntpOpened;
        Interpolation*              newIntp;
        char                        newIntpBuff[INPUT_BUFFER_LEN];

        bool                        show_demo_window = true;    //

        // methods
        void                        refreshGraphValues(uint32_t _steps = 1000);

        // functions for converting from/to plane and screen coordinate spaces
        inline float                planeToScreenSpaceX(float _v) { return fmap(_v, rangeMin.x, rangeMax.x, 0, graphSize.x - 1, false) + 1; };
        inline float                planeToScreenSpaceY(float _v) { return fmap(_v, rangeMax.y, rangeMin.y, 0, graphSize.y - 1, false) + 1; };
        inline float                screenToPlaneSpaceX(float _v) { return fmap(_v, 0, graphSize.x - 1, rangeMin.x, rangeMax.x, false); };
        inline float                screenToPlaneSpaceY(float _v) { return fmap(_v, 0, graphSize.y - 1, rangeMin.y, rangeMax.y, false); };

        // functions for drawing widgets on the screen
        inline bool                 isGraphPosX(float _x) { return _x >= graphPos.x && _x < graphPos.x + graphSize.x; };
        inline bool                 isGraphPosY(float _y) { return _y >= graphPos.y && _y < graphPos.y + graphSize.y; };
        void                        drawPopupNewInterpolation();
        void                        drawListboxDataPoints(std::vector<ImVec2>& _data, ImVec2** _pointSelected);
        bool                        drawListitemPoint(ImVec2& _p, bool* _isSelected);
        void                        drawPanelLeft();
        void                        drawPanelMiddle();
        void                        drawPanelBottom();
        void                        drawGraphAxes(ImDrawList* _dl, const ImVec4& _color);
        void                        drawGraphPoint(ImDrawList * _dl, ImVec2& _p, bool _isSelected, ImVec4& _color, bool _square = false, float _radius = 4.0f);
        void                        drawGraphCurve(std::vector<float>& _yValues, float _yMin, float _yMax, const ImVec4& _color, const ImVec2& _size);
        void                        drawOption(const char* _label, GraphOption& _opt);

        // imgui helpers
        static void                 PushDisabled();
        static void                 PopDisabled();
    };
}

#endif // RENDERER_H_

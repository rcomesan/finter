#ifndef RENDERER_H_
#define RENDERER_H_

#include "interpolator.h"
#include "defines.h"

namespace finter
{
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

        bool                        newIntpOpened;
        Interpolation*              newIntp;
        char                        newIntpBuff[INPUT_BUFFER_LEN];

        bool                        show_demo_window = true;    //

        // methods
        void DrawPopupNewInterpolation();
        void DrawListboxDataPoints(std::vector<ImVec2>& _data);
        void DrawPanelLeft();
        void DrawPanelMiddle();
    };
}

#endif // RENDERER_H_

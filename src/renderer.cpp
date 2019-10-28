#include "renderer.h"
#include "interpolator.h"
#include "imgui.h"

namespace finter
{
    Renderer::Renderer()
    {
        // set our theme
        ImGui::StyleColorsDark();

        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowBorderSize = 0;
        s.WindowRounding = 0;
        s.ChildRounding = 0;
        s.PopupRounding = 0;
        s.FrameRounding = 0;
        s.ScrollbarRounding = 0;
        s.GrabRounding = 0;
        s.TabRounding = 0;
    }

    Renderer::~Renderer()
    {

    }

    void Renderer::Draw()
    {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        DrawPanelLeft();
        DrawPanelMiddle();


    }

    void Renderer::DrawPanelLeft()
    {
       static bool selected;

        ImGuiWindowFlags wflags = ImGuiWindowFlags_None
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_AlwaysUseWindowPadding
            | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(LEFT_PANEL_WIDTH, VIEWPORT_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin("Main", nullptr, wflags);

        if (ImGui::Button("New Interpolation", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
        {
            newIntp = new Interpolation();
            newIntpOpened = true;
            MEM_ZERO(newIntpBuff);

            ImGui::OpenPopup("New Interpolation");
        }
        DrawPopupNewInterpolation();

        if (ImGui::ListBoxHeader("Interpolations"))
        {
            for (uint32_t i = 0; i < interpolator.getCount(); i++)
            {
                Interpolation& interpolation = interpolator.getInterpolations()[i];

                ImGui::PushID(&interpolation);
                selected = (curIntp == &interpolation);
                if (ImGui::Selectable(interpolation.name, &selected))
                {
                    curIntp = &interpolation;
                }
                ImGui::PopID();
            }

            ImGui::ListBoxFooter();
        }

        ImGui::Separator();

        if (nullptr != curIntp)
        {
            ImGui::Text("Selected Interpolation:");
            ImGui::InputText("Name", curIntp->name, sizeof(curIntp->name));
            DrawListboxDataPoints(curIntp->datapoints);

            ImGui::Separator();

            if (ImGui::BeginTabBar("Polynomials", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Lagrange"))
                {
                    ImGui::Text("This is the Lagrange tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Newton-Gregory"))
                {
                    ImGui::Text("This is the Newton-Gregory tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        else
        {
            ImGui::Text("Selected Interpolation: none");
        }

        ImGui::End();
    }

    void Renderer::DrawPanelMiddle()
    {
        ImGuiWindowFlags wflags = ImGuiWindowFlags_None
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_AlwaysUseWindowPadding
            | ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImGui::SetNextWindowPos(ImVec2(LEFT_PANEL_WIDTH, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(VIEWPORT_WIDTH - LEFT_PANEL_WIDTH, VIEWPORT_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graph", nullptr, wflags);

        struct Funcs
        {
            static float Demo1(void* _userData, int32_t _i) { return powf(_i, 3); }
        };

        ImGui::PushItemWidth(-15.0f);
        ImGui::PlotLines("##Polynomial", Funcs::Demo1, NULL, 100, -50, NULL, -125000.0f, 125000.0f, ImVec2(0, 500));
        ImGui::PopItemWidth();

        ImGui::End();
    }

    void Renderer::DrawPopupNewInterpolation()
    {
        ImGuiWindowFlags wflags = ImGuiWindowFlags_None
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_AlwaysUseWindowPadding;

        ImGui::SetNextWindowSize(ImVec2(500, 0));

        if (ImGui::BeginPopupModal("New Interpolation", &newIntpOpened, wflags))
        {
            ImGui::InputText("Name", newIntp->name, sizeof(newIntp->name));
            ImGui::InputTextMultiline("Data", newIntpBuff, INPUT_BUFFER_LEN);
            
            if (ImGui::Button("Cancel", ImVec2(248, 0)))
            {
                newIntpOpened = false;
            }

            ImGui::SameLine(0, 4);

            if (ImGui::Button("Ok", ImVec2(248, 0)))
            {
                if (!Interpolation::parseData(newIntpBuff, newIntp->datapoints))
                {
                    
                }
                else
                {
                    newIntpOpened = false;
                    interpolator.add(*newIntp);
                }
            }

            if (!newIntpOpened)
            {
                ImGui::CloseCurrentPopup();
                delete newIntp;
            }

            ImGui::EndPopup();
        }
    }

    void Renderer::DrawListboxDataPoints(std::vector<ImVec2>& _data)
    {
        static char buff[255];
        static bool selected;
        if (ImGui::ListBoxHeader("Datapoints"))
        {
            for (auto it = _data.begin(); it != _data.end(); ++it)
            {
                ImVec2& point = *it;

                ImGui::PushID(&point);
                snprintf(buff, sizeof(buff), "x=%.2f y=%.2f", point.x, point.y);
                selected = false;
                if (ImGui::Selectable(buff, &selected))
                {
                }
                ImGui::PopID();

            }
            ImGui::ListBoxFooter();
        }
    }
}
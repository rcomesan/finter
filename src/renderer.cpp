#include "renderer.h"
#include "interpolator.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "math.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <inttypes.h>
#include <algorithm>

namespace finter
{
    Renderer::Renderer(ID3D11Device* _pd3dDevice)
    {
        device = _pd3dDevice;

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

        goptLagrange = { true, ImVec4(1.0f, 1.0f, 0.0f, 1.0f) };
        goptNewtonPr = { true , ImVec4(0.0f, 1.0f, 1.0f, 1.0f) };
        goptNewtonRe = { true, ImVec4(0.0f, 1.0f, 0.0f, 1.0f) };
        goptAxes = { true, ImVec4(1.0f, 1.0f, 1.0f, 1.0f) };
        goptDatapoints = { true, ImVec4(1.0f, 0.0f, 1.0f, 1.0f) };
        goptCurPoint = { true, ImVec4(1.0f, 1.0f, 1.0f, 1.0f) };

        rangeMin = { -100, -100 };
        rangeMax = { 100, 100 };
        curVariant = Interpolation_Lagrange;

        latexLagrange.steps.reserve(MAX_DATAPOINTS);
        latexNewtonPr.steps.reserve(MAX_DATAPOINTS);
        latexNewtonRe.steps.reserve(MAX_DATAPOINTS);
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::Draw()
    {
        drawPanelLeft();
        drawPanelMiddle();
        drawPanelBottom();
    }

    void Renderer::drawPanelLeft()
    {
        bool selected = false;
        bool tabChanged = false;

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
        ImGui::SetNextWindowSize(ImVec2(LEFT_PANEL_WIDTH, LEFT_PANEL_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin("Main", nullptr, wflags);

        if (ImGui::Button("New Interpolation", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
        {
            newIntp = new Interpolation();
            newIntpOpened = true;
            ZERO_MEM(newIntpBuff);

            ImGui::OpenPopup("New Interpolation");
        }
        drawPopupNewInterpolation();

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
                    Renderer::refreshGraphValues();
                    Renderer::refreshLatexFormulas(curVariant, false);
                    Renderer::resetView();
                }
                ImGui::PopID();
            }
            ImGui::ListBoxFooter();
        }

        ImGui::Separator();

        if (nullptr != curIntp)
        {
            ImGui::InputText("Name", curIntp->name, sizeof(curIntp->name));
            drawListboxDataPoints(curIntp->datapoints, &curIntp->datapointSelected);
            ImGui::LabelText("Equidistant", curIntp->datapointsEquidistant ? "Yes" : "No");

            ImGui::Separator();

            if (ImGui::BeginTabBar("Polynomials", ImGuiTabBarFlags_None))
            {
                if (Renderer::drawTab("Lagrange", latexLagrange))
                {
                    tabChanged = true;
                    curVariant = Interpolation_Lagrange;
                    curPoint.y = curIntp->evalLagrange(curPoint.x);
                }

                if (Renderer::drawTab("Newton Progressive", latexNewtonPr))
                {
                    tabChanged = true;
                    curVariant = Interpolation_NewtonPr;
                    curPoint.y = curIntp->evalNewtonPr(curPoint.x);
                }
                
                if (Renderer::drawTab("Newton Regressive", latexNewtonRe))
                {
                    tabChanged = true;
                    curVariant = Interpolation_NewtonRe;
                    curPoint.y = curIntp->evalNewtonRe(curPoint.x);
                }

                if (tabChanged)
                    Renderer::refreshLatexFormulas(curVariant, false);


                ImGui::EndTabBar();
            }
        }
        else
        {
            ImGui::Text("Selected Interpolation: none");
        }

        ImGui::End();
    }

    void Renderer::drawPanelMiddle()
    {
        if (curIntp == nullptr) return;

        ImGuiWindowFlags wflags = ImGuiWindowFlags_None
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_AlwaysUseWindowPadding
            | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::SetNextWindowPos(ImVec2(LEFT_PANEL_WIDTH, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(MIDDLE_PANEL_WIDTH, MIDDLE_PANEL_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(MIDDLE_PANEL_WIDTH, MIDDLE_PANEL_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graph", nullptr, wflags);

        graphPos = ImGui::GetCursorScreenPos();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.025f));
        ImGui::PushItemWidth(0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::BeginChild("graph-child-container", ImVec2(0, GRAPH_HEIGHT), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float w = ImGui::GetContentRegionAvailWidth();

        ImDrawList* dl = ImGui::GetWindowDrawList();        

        if (goptAxes.visible)
            drawGraphAxes(dl, goptAxes.color);

        if (goptLagrange.visible)
        {
            Renderer::drawGraphCurve(gdataLagrange.yS, rangeMin.y, rangeMax.y, goptLagrange.color, ImVec2(w, GRAPH_HEIGHT));
            Renderer::drawGraphPoint(dl, mousePointLagrange, false, goptLagrange.color);
        }
   
        if (goptNewtonPr.visible)
        {
            Renderer::drawGraphCurve(gdataNewtonPr.yS, rangeMin.y, rangeMax.y, goptNewtonPr.color, ImVec2(w, GRAPH_HEIGHT));
            Renderer::drawGraphPoint(dl, mousePointNewtonPr, false, goptNewtonPr.color);
        }

        if (goptNewtonRe.visible)
        {
            Renderer::drawGraphCurve(gdataNewtonRe.yS, rangeMin.y, rangeMax.y, goptNewtonRe.color, ImVec2(w, GRAPH_HEIGHT));
            Renderer::drawGraphPoint(dl, mousePointNewtonRe, false, goptNewtonRe.color);
        }

        if (goptDatapoints.visible)
        {
            for (auto it = curIntp->datapoints.begin(); it != curIntp->datapoints.end(); ++it)
            {
                drawGraphPoint(dl, *it, curIntp->datapointSelected == &(*it), goptDatapoints.color);
            }
        }
        if (goptCurPoint.visible) drawGraphPoint(dl, curPoint, false, goptCurPoint.color, true, 6.0f);

        if (ImGui::IsItemHovered())
        {
            mousePointLagrange.x = screenToPlaneSpaceX(ImGui::GetMousePos().x - graphPos.x);
            mousePointNewtonPr.x = mousePointLagrange.x;
            mousePointNewtonRe.x = mousePointLagrange.x;

            mousePointLagrange.y = curIntp->evalLagrange(mousePointLagrange.x);
            mousePointNewtonPr.y = curIntp->evalNewtonPr(mousePointLagrange.x);
            mousePointNewtonRe.y = curIntp->evalNewtonRe(mousePointLagrange.x);
                
            ImGui::SetTooltip(" L(%.4f) = %.4f\nNp(%.4f) = %.4f\nNr(%.4f) = %.4f",
                mousePointLagrange.x, mousePointLagrange.y,
                mousePointNewtonPr.x, mousePointNewtonPr.y,
                mousePointNewtonRe.x, mousePointNewtonRe.y);
        }
        ImGui::EndChild();
        ImGui::PopItemWidth();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();

        ImGui::End();
    }

    void Renderer::drawPanelBottom()
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

        ImGui::SetNextWindowPos(ImVec2(LEFT_PANEL_WIDTH, MIDDLE_PANEL_HEIGHT), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(BOTTOM_PANEL_WIDTH, BOTTOM_PANEL_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin("Options", nullptr, wflags);

        ImGui::PushItemWidth(0.6 * BOTTOM_PANEL_WIDTH);
        if (NULL == curIntp) Renderer::pushDisabled();
        ImGui::BeginGroup();
        if (ImGui::Button("Reset View"))
        {
            Renderer::resetView();
        }

        if (ImGui::DragFloatRange2("range for x", &rangeMin.x, &rangeMax.x))
        {
            Renderer::refreshGraphValues();
            Renderer::resetView();
        }
        if (ImGui::DragFloatRange2("range for y", &rangeMin.y, &rangeMax.y))
        {
            Renderer::refreshGraphValues();
        }
        
        ImGui::EndGroup();
        if (NULL == curIntp) Renderer::popDisabled();
        ImGui::PopItemWidth();

        ImGui::SameLine(0.0f, 125.0f);

        ImGui::BeginGroup();
        drawOption("Lagrange", goptLagrange);
        drawOption("Newton Pr", goptNewtonPr);
        drawOption("Newton Re", goptNewtonRe);
        ImGui::Separator();
        drawOption("Axes", goptAxes);
        drawOption("Datapoints", goptDatapoints);
        drawOption("Current Point", goptCurPoint);
        ImGui::EndGroup();

        ImGui::End();
    }

    bool Renderer::drawTab(const char* _name, LatexData& _latex)
    {
        bool active = false;

        if (ImGui::BeginTabItem(_name))
        {
            ImGui::SetWindowFontScale(1.0f);

            int32_t degree = i32max(0, curIntp->datapoints.size() - 1);
            ImGui::Text("Polynomial of Degree %" PRId32, degree);

            Renderer::drawLatex(_latex.px.c_str());

            if (ImGui::Button("Show Step-by-Step Solution", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.0f)))
            {
                stepByStepOpened = true;
                Renderer::refreshLatexFormulas(curVariant, true);
                ImGui::OpenPopup("Step-by-Step Solution");
            }
            drawPopupStepByStepSolution(_name, _latex);

            ImGui::Separator();

            ImGui::Text("Evaluate");
            active = ImGui::SliderFloat("Input", &curPoint.x, rangeMin.x, rangeMax.x, "P(%.4f)");
            ImGui::DragFloat("Output", &curPoint.y, 0.0f, 0.0f, 0.0f, "%.4f", 0.0f);
            ImGui::Separator();

            ImGui::EndTabItem();
        }
        active = active | ImGui::IsItemActivated();

        return active;
    }

    void Renderer::drawPopupNewInterpolation()
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
                    // TODO show error to the user
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

    void Renderer::drawPopupStepByStepSolution(const char* _name, LatexData& _data)
    {
        ImGuiWindowFlags wflags = ImGuiWindowFlags_None
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_AlwaysUseWindowPadding;

        ImGui::SetNextWindowSize(ImVec2(800, 0));

        if (ImGui::BeginPopupModal("Step-by-Step Solution", &stepByStepOpened, wflags))
        {
            ImGui::Text(_name);
            Renderer::drawLatex(_data.px.c_str());

            ImGui::Text("Formula");
            Renderer::drawLatex(_data.steps[0].c_str());

            ImGui::Text("Steps");
            for (uint32_t i = 1; i < _data.steps.size(); i++)
            {
                ImGui::Separator();
                Renderer::drawLatex(_data.steps[i].c_str());
            }

            if (!stepByStepOpened)
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void Renderer::drawListboxDataPoints(std::vector<ImVec2>& _data, ImVec2** _pointSelected)
    {
        bool isSelected = false;     // true if a given list item is currently selected
        bool isModified = false;     // true if our dataset was modified and a recalculation is needed

        if (ImGui::Button("Clear"))
        {
            isModified = true;
            _data.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove"))
        {
            isModified = true;
            _data.erase(
                std::remove_if(_data.begin(), _data.end(), [_pointSelected](ImVec2& _v) { return &_v == (*_pointSelected); }),
                _data.end()
            );
        }
        ImGui::SameLine();
        if (ImGui::Button("Add"))
        {
            isModified = true;
            _data.emplace_back(0.0f, 0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Rnd"))
        {
            isModified = true;
            static uint32_t rgX = (uint32_t)(abs(rangeMax.x - rangeMin.x));
            _data.emplace_back(rangeMin.x + rand() % rgX, rangeMin.x + rand() % rgX);
        }

        if (ImGui::ListBoxHeader("Datapoints"))
        {
            for (auto it = _data.begin(); it != _data.end(); ++it)
            {
                isSelected = &(*it) == (*_pointSelected);
                isModified = drawListitemPoint(*it, &isSelected) || isModified;

                if (isSelected)
                {
                    (*_pointSelected) = &(*it);
                }
            }
            ImGui::ListBoxFooter();
        }

        if (isModified)
        {
            curIntp->recalculateDiffs();
            curPoint.y = Lagrange::eval(curIntp->datapoints, curPoint.x);
            Renderer::refreshGraphValues();
            Renderer::refreshLatexFormulas(curVariant, false);
            Renderer::resetView();
        }
    }

    bool Renderer::drawListitemPoint(ImVec2& _p, bool* _isSelected)
    {
        static char buff[255];
        bool modified = false;

        ImGui::PushID(&_p);

        snprintf(buff, sizeof(buff), "(%.4f, %.4f)", _p.x, _p.y);
        *_isSelected = ImGui::Selectable(buff, _isSelected);

        if (ImGui::BeginPopupContextItem("point-context-menu"))
        {
            *_isSelected = true;

            modified = false
                || ImGui::SliderFloat("x", &_p.x, rangeMin.x, rangeMax.x, "%.4f")
                || ImGui::InputFloat("f(x)", &_p.y, 0.5f, 1.0f, 0);

            ImGui::EndPopup();
        }

        ImGui::PopID();

        return modified;
    }

    void Renderer::drawGraphCurve(std::vector<float>& _yValues, float _yMin, float _yMax, const ImVec4& _color, const ImVec2& _size)
    {
        if (_yValues.size() > 0)
        {
            ImGui::SetCursorScreenPos(graphPos);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_PlotLines, _color);
            ImGui::PlotLines("##polynomial-curve", &_yValues[0], _yValues.size(), 0, NULL, _yMin, _yMax, _size);
            ImGui::PopStyleColor(2);

            graphSize = ImGui::GetItemRectSize();
        }
    }

    void Renderer::drawGraphAxes(ImDrawList* _dl, const ImVec4& _color)
    {
        static ImVec2 p1;
        static ImVec2 p2;

        float xAxisPosY = planeToScreenSpaceY(0) + graphPos.y;
        if (isGraphPosY(xAxisPosY))
        {
            p1 = { graphPos.x,  xAxisPosY };
            p2 = { graphPos.x + graphSize.x, xAxisPosY };

            _dl->AddLine(p1, p2, ImColor(_color));
        }

        float yAxisPosX = planeToScreenSpaceX(0) + graphPos.x;
        if (isGraphPosX(yAxisPosX))
        {
            p1 = { yAxisPosX, graphPos.y };
            p2 = { yAxisPosX, graphPos.y + graphSize.y };

            _dl->AddLine(p1, p2, ImColor(_color));
        }
    }

    void Renderer::drawGraphPoint(ImDrawList* _dl, ImVec2& _p, bool _isSelected, ImVec4& _color, bool _square, float _radius)
    {
        const float _radiusHalf = _radius * 0.5f;
        static ImVec2 screenPos;
        screenPos.x = planeToScreenSpaceX(_p.x) + graphPos.x;
        screenPos.y = planeToScreenSpaceY(_p.y) + graphPos.y;
        
        if (isGraphPosX(screenPos.x) && isGraphPosY(screenPos.y))
        {
            if (_square)
            {
                static ImVec2 screenPosMax;
                static ImVec2 screenPosMin;
                screenPosMin = { screenPos.x - _radiusHalf, screenPos.y - _radiusHalf };
                screenPosMax = { screenPos.x + _radiusHalf, screenPos.y + _radiusHalf };

                _dl->AddRectFilled(screenPosMin, screenPosMax, ImColor(_color));
            }
            else
            {
                _dl->AddCircleFilled(screenPos, _radius, ImColor(_color));
            }

            if (_isSelected)
                _dl->AddCircle(screenPos, _radius * 2.0f, IM_COL32(255, 255, 255, 155));
        }
    }

    void Renderer::drawOption(const char* _label, GraphOption& _opt)
    {
        ImGui::ColorEdit4(_label, &_opt.color.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine(0, 4.0f);
        ImGui::Checkbox(_label, &_opt.visible);
    }

    void Renderer::drawLatex(const char* _latex)
    {
        auto tex = texMap.find(std::string(_latex));
        if (tex == texMap.end())
        {
            TextureData* texd = new TextureData();

            latex.to_png(_latex, Latex::tmp_png_path());
            if (Renderer::loadTextureFromFile(Latex::tmp_png_path().c_str(), &texd->srv, &texd->w, &texd->h))
            {
                texd->latex = std::string(_latex);
                texMap.insert(std::pair<std::string, TextureData*>(std::string(_latex), texd));

                if (texLru.size() >= TEXTURES_CACHE_SIZE)
                {
                    auto it = texLru.end();
                    for (int32_t i = 0; i < texLru.size() - TEXTURES_CACHE_SIZE + 1; i++)
                    {
                        texLru.erase(it);
                        texMap.erase((*it)->latex);

                        (*it)->srv->Release();
                        delete (*it);

                        it--;
                    }
                }
                texLru.push_front(texd);
                texd->lruIt = texLru.begin();
                tex = texMap.find(std::string(_latex));
            }
        }
        else
        {
            // cache hit. bring it to the front
            if (texLru.begin() != tex->second->lruIt)
            {
                texLru.splice(texLru.begin(), texLru, tex->second->lruIt, std::next(tex->second->lruIt));
            }
        }

        ImGui::PushID(tex->second->srv);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::BeginChild("latex-formula-container", ImVec2(ImGui::GetContentRegionAvailWidth(), tex->second->h + 15), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Image(tex->second->srv, ImVec2(tex->second->w, tex->second->h));
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
    
    bool Renderer::loadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
    {
        // Load from disk into a raw RGBA buffer
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = image_width;
        desc.Height = image_height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D *pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = image_data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        device->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        device->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
        pTexture->Release();

        *out_width = image_width;
        *out_height = image_height;
        stbi_image_free(image_data);

        return true;
    }

    void Renderer::pushDisabled()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.35);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }

    void Renderer::popDisabled()
    {
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
    }

    void Renderer::resetView()
    {
        rangeMin.y = fmin(fmin(gdataLagrange.min, gdataNewtonPr.min), gdataNewtonRe.min);
        rangeMax.y = fmax(fmax(gdataLagrange.max, gdataNewtonPr.max), gdataNewtonRe.max);
        refreshGraphValues();
    }

    void Renderer::refreshGraphValues(uint32_t _steps)
    {
        if (NULL == curIntp) return;

        float increment = (rangeMax.x - rangeMin.x) / _steps;
        float x = rangeMin.x;
        float y = 0.0f;

        curIntp->datapointsEquidistant = true;
        float dist;
        if (curIntp->datapoints.size() >= 2)
        {
            dist = abs(curIntp->datapoints[1].x - curIntp->datapoints[0].x);

            for (uint32_t i = 0; i < curIntp->datapoints.size() - 1; i++)
            {
                if (dist != abs(curIntp->datapoints[i + 1].x - curIntp->datapoints[i].x))
                {
                    curIntp->datapointsEquidistant = false;
                    break;
                }
            }
        }
        
        gdataLagrange.yS.clear();
        gdataLagrange.yS.reserve(_steps);
        gdataLagrange.min = FLT_MAX;
        gdataLagrange.max = 0;

        gdataNewtonPr.yS.clear();
        gdataNewtonPr.yS.reserve(_steps);
        gdataNewtonPr.min = FLT_MAX;
        gdataNewtonPr.max = 0;

        gdataNewtonRe.yS.clear();
        gdataNewtonRe.yS.reserve(_steps);
        gdataNewtonRe.min = FLT_MAX;
        gdataNewtonRe.max = 0;

        for (int32_t i = 0; i < _steps; i++)
        {
            y = curIntp->evalLagrange(x);
            gdataLagrange.min = fmin(gdataLagrange.min, y);
            gdataLagrange.max = fmax(gdataLagrange.max, y);
            gdataLagrange.yS.push_back(y);

            y = curIntp->evalNewtonPr(x);
            gdataNewtonPr.min = fmin(gdataNewtonPr.min, y);
            gdataNewtonPr.max = fmax(gdataNewtonPr.max, y);
            gdataNewtonPr.yS.push_back(y);

            y = curIntp->evalNewtonRe(x);
            gdataNewtonRe.min = fmin(gdataNewtonRe.min, y);
            gdataNewtonRe.max = fmax(gdataNewtonRe.max, y);
            gdataNewtonRe.yS.push_back(y);

            x += increment;
        }
    }

    void Renderer::refreshLatexFormulas(InterpolationVariant _variant, bool _steps)
    {
        uint32_t s = 0;

        bool newtonPro = Interpolation_NewtonPr == _variant;
        LatexData& _dataNw = newtonPro ? latexNewtonPr : latexNewtonRe;

        if (_steps)
        {
            if (Interpolation_Lagrange == _variant)
            {
                latexLagrange.steps.resize(curIntp->datapoints.size() + 1);

                Lagrange::latexFormula(curIntp->datapoints, latexLagrange.steps[0]);

                for (uint32_t i = 0; i < curIntp->datapoints.size(); i++)
                {
                    Lagrange::latexLx(curIntp->datapoints, i, latexLagrange.steps[i + 1]);
                }
            }
            else
            {
                _dataNw.steps.resize(500);
                Newton::latexFormula(curIntp->datapoints, curIntp->diffs, newtonPro, _dataNw.steps[0]);

                s = 1;
                for (uint32_t diffOrder = 1; diffOrder < curIntp->diffs.size(); diffOrder++)
                {
                    for (uint32_t diffIndex = 0; diffIndex < curIntp->diffs[diffOrder].size(); diffIndex++)
                    {
                        Newton::latexFx(curIntp->datapoints, curIntp->diffs, newtonPro, diffIndex, diffIndex + diffOrder, _dataNw.steps[s++]);
                    }
                }
                _dataNw.steps.resize(s);
            }
        }
        else
        {
            if (Interpolation_Lagrange == _variant)
            {
                Lagrange::latexPx(curIntp->datapoints, latexLagrange.px);
            }
            else
            {
                Newton::latexPx(curIntp->datapoints, curIntp->diffs, newtonPro, _dataNw.px);
            }
        }
    }
}
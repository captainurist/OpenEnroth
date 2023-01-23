#include "GUI/GUIProgressBar.h"

#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Tables/IconFrameTable.h"

#include "Library/Random/Random.h"
#include "Utility/IndexedArray.h"

static const IndexedArray<const char *, PartyAlignment_Good, PartyAlignment_Evil> ProgressBarResourceByAlignment = {
    {PartyAlignment_Good, "bardata-b"},
    {PartyAlignment_Neutral, "bardata"},
    {PartyAlignment_Evil, "bardata-c"}
};

GUIProgressBar *pGameLoadingUI_ProgressBar = new GUIProgressBar();

GUIProgressBar::GUIProgressBar() {
    progressbar_dungeon = nullptr;
    progressbar_loading = nullptr;
    loading_bg = nullptr;
}

bool GUIProgressBar::Initialize(Type type) {
    if (loading_bg) {
        return false;
    }

    Release();

    switch (type) {
        case TYPE_None:
            return true;

        case TYPE_Box:
        case TYPE_Fullscreen:
            break;

        default:
            Error("Invalid GUIProgressBar type: %u", type);
    }

    uType = type;

    if (uType == TYPE_Fullscreen) {
        loading_bg = assets->GetImage_PCXFromIconsLOD(StringPrintf("loading%d.pcx", Random(5) + 1));

        uProgressCurrent = 0;
        uX = 122;
        uY = 151;
        uWidth = 449;
        uHeight = 56;
        uProgressMax = 26;

        progressbar_loading = assets->GetImage_Alpha("loadprog");
        Draw();
        return true;
    } else {
        progressbar_dungeon = assets->GetImage_ColorKey(ProgressBarResourceByAlignment[pParty->alignment], render->teal_mask_16);
    }

    uProgressCurrent = 0;
    uProgressMax = 26;
    Draw();
    return true;
}

void GUIProgressBar::Reset(uint8_t uMaxProgress) {
    uProgressCurrent = 0;
    uProgressMax = uMaxProgress;
}

void GUIProgressBar::Progress() {
    uProgressCurrent = std::min((uint8_t)(uProgressCurrent + 1), uProgressMax);
    Draw();
}

void GUIProgressBar::Release() {
    if (loading_bg != nullptr) {
        loading_bg->Release();
        loading_bg = nullptr;
    }

    if (progressbar_loading != nullptr) {
        progressbar_loading->Release();
        progressbar_loading = nullptr;
    }

    if (progressbar_dungeon != nullptr) {
        progressbar_dungeon->Release();
        progressbar_dungeon = nullptr;
    }

    uType = TYPE_None;
}

void GUIProgressBar::Draw() {
    // render->BeginSceneD3D();
    render->BeginScene();
    //render->ClearBlack();

    if (uType != TYPE_Fullscreen) {
        engine->DrawGUI();
        GUI_UpdateWindows();
        pParty->UpdatePlayersAndHirelingsEmotions();

        render->DrawTextureAlphaNew(80 / 640.0f, 122 / 480.0f, progressbar_dungeon);
        render->DrawTextureAlphaNew(100 / 640.0f, 146 / 480.0f, pIconsFrameTable->GetFrame(uIconID_TurnHour, 0)->GetTexture());
        render->FillRectFast(174, 164, floorf(((double)(113 * uProgressCurrent) / (double)uProgressMax) + 0.5f), 16, 0xF800);
    } else {
        if (loading_bg) {
            render->DrawTextureNew(0, 0, loading_bg);
        }
        render->SetUIClipRect(172, 459, (int)((double)(300 * uProgressCurrent) / (double)uProgressMax) + 172, 471);
        render->DrawTextureAlphaNew(172 / 640.0f, 459 / 480.0f, progressbar_loading);
        render->ResetUIClipRect();
    }

    render->EndScene();
    render->Present();
}

bool GUIProgressBar::IsActive() {
    return uType != TYPE_None;
}

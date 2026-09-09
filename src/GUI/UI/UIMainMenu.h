#pragma once

#include <functional>

#include "GUI/GUIWindow.h"

class GUIWindow_MainMenu : public GUIWindow {
 public:
    GUIWindow_MainMenu();
    virtual ~GUIWindow_MainMenu();

    virtual void Update() override;

    void processMessage(UIMessageType messageType);

 protected:
    GUIButton *pBtnExit;
    GUIButton *pBtnCredits;
    GUIButton *pBtnLoad;
    GUIButton *pBtnNew;

    std::shared_ptr<GraphicsImage> main_menu_background;

    std::shared_ptr<GraphicsImage> ui_mainmenu_new;
    std::shared_ptr<GraphicsImage> ui_mainmenu_load;
    std::shared_ptr<GraphicsImage> ui_mainmenu_credits;
    std::shared_ptr<GraphicsImage> ui_mainmenu_exit;
};

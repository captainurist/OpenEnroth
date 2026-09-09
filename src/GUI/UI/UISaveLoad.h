#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Save : public GUIWindow {
 public:
    GUIWindow_Save();
    virtual ~GUIWindow_Save() {}

    virtual void Update() override;

 protected:
    std::shared_ptr<GraphicsImage> saveload_ui_save_up;
    std::shared_ptr<GraphicsImage> saveload_ui_loadsave;
    std::shared_ptr<GraphicsImage> saveload_ui_saveu;
    std::shared_ptr<GraphicsImage> saveload_ui_x_u;
};

class GUIWindow_Load : public GUIWindow {
 public:
    explicit GUIWindow_Load(bool ingame);
    virtual ~GUIWindow_Load() {}

    virtual void Update() override;

    void slotSelected(int slotIndex);
    void loadButtonPressed();
    void downArrowPressed(int maxSlots);
    void upArrowPressed();
    void cancelButtonPressed();
    void scroll(int maxSlots);
    void quickLoad();

 protected:
    bool isLoadSlotClicked{};
    std::shared_ptr<GraphicsImage> main_menu_background;

    std::shared_ptr<GraphicsImage> saveload_ui_load_up;
    std::shared_ptr<GraphicsImage> saveload_ui_loadsave;
    std::shared_ptr<GraphicsImage> saveload_ui_loadu;
    std::shared_ptr<GraphicsImage> saveload_ui_x_u;
};

#include <cstdlib>

#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Time/Timer.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIBooks.h"

#include "Media/Audio/AudioPlayer.h"

void GUIWindow_Book::Release() {
    ui_book_map_frame.reset();
    ui_book_quest_div_bar.reset();
    ui_book_button8_off.reset();
    ui_book_button8_on.reset();
    ui_book_button7_off.reset();
    ui_book_button7_on.reset();
    ui_book_button6_off.reset();
    ui_book_button6_on.reset();
    ui_book_button5_off.reset();
    ui_book_button5_on.reset();
    ui_book_button4_off.reset();
    ui_book_button4_on.reset();
    ui_book_button3_off.reset();
    ui_book_button3_on.reset();
    ui_book_button2_off.reset();
    ui_book_button2_on.reset();
    ui_book_button1_off.reset();
    ui_book_button1_on.reset();

    pAudioPlayer->playUISound(SOUND_closebook);

    if (pChildBooksOverlay) {
        pChildBooksOverlay->Release();
    }

    GUIWindow::Release();
}

GUIWindow_Book::GUIWindow_Book() : GUIWindow(WINDOW_Book, {0, 0}, render->GetRenderDimensions()) {
    initializeFonts();
    CreateButton({475, 445}, {158, 34}, 1, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE));
    current_screen_type = SCREEN_BOOKS;
    pEventTimer->setPaused(true);
}

void GUIWindow_Book::initializeFonts() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_book_map_frame = assets->getImage_Alpha("mapbordr");

    if (!assets->pFontBookCalendar)
        assets->pFontBookCalendar = GUIFont::LoadFont("book.fnt");
    if (!assets->pFontBookTitle)
        assets->pFontBookTitle = GUIFont::LoadFont("book2.fnt");
    if (!assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow = GUIFont::LoadFont("autonote.fnt");
    if (!assets->pFontBookLloyds)
        assets->pFontBookLloyds = GUIFont::LoadFont("spell.fnt");
}

void GUIWindow_Book::bookButtonClicked(BookButtonAction action) {
    _bookButtonClicked = 10; // TODO(pskelton): make a constant for this magic number
    _bookButtonAction = action;
}

GUIWindow_BooksButtonOverlay::GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint) :
    GUIWindow(WINDOW_BooksButtonOverlay, position, dimensions, hint),
    _button(button)
{}

void GUIWindow_BooksButtonOverlay::Update() {
    render->DrawTextureNew(uFrameY / 640.0f, uFrameX / 480.0f, _button->vTextures[0].get());
}

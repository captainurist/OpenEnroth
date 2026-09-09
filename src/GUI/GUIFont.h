#pragma once

#include <array>
#include <vector>
#include <string>
#include <memory>

#include "Library/Color/Color.h"
#include "Library/Image/Palette.h"
#include "Library/Geometry/Point.h"
#include "Library/LodFormats/LodFont.h"

class GUIWindow;
class GraphicsImage;

// So, functionality:
// - draw text with word wrap in box.
// - get height for width.
// - draw center-justified text.

// BUT layout engine doesn't even need to know anything about fonts. Layout engine just lays out boxes.
//
// Layout:
//  hasHeightForWidth.
//  heightForWidth => function(Box*, width).
//  minWidth, minHeight.
//  maxWidth, maxHeight.
//  sizeHint.
//  data -> void*.
//  type() -> BoxType.
//  parent()
//  isDirty() => polish() / invalidate()
//  children() - need to be here so that the tree is externally traversable.
//  visible() - can hide an element & this is handled in a sane way
//
// LayoutBox:
//  setMinWidth, setMaxWidth
//  setMinHeigh, setMaxHeight
//  setHeightForWidth
//
// Label is a LeafBox. maxWidth is when expanding makes no sense (text fits), minWidth is shortest word, minHeight is text height, maxHeight corresponds to ...
// preferredSize is one-liner size.
//
// PinnedLayout:
//  addChild(x, y, w, h, Box).
//  children()
//
// MarginsLayout:
//  margins()
//  setMargins()
//  child()
//  setChild()
//
// LinearLayout:
//  spacing()
//  addChild(stretch, Box)
//  addSpacer(stretch)
//  - sizehint is sum of sizehints pretty much, stretch doesn't do anything.
//
// TableLayout:
//  rows() cols()
//  rowSpacing() colSpacing()
//  addChild(x, y, Box)
//
// ScrollLayout:
//  prefferedSize == content preferredSize
//  minSize = 0
//  offset / setOffset
//  layout() lays out child at offset using child's prefSize?
//  child()
//
// What tableBox does (inspiration here https://codebrowser.dev/qt5/qtbase/src/widgets/kernel/qlayoutengine.cpp.html#_Z9qGeomCalcR7QVectorI13QLayoutStructEiiiii).
//  minWidth => get minWidth of all.
//  minHeight => same.
//  maxWidth => same.
//  maxHeight => same.
//  layout(geometry)
//  - if we can fit minsize (we always can) but not prefsize =>
//    - use the water-filling-the-mountain-range algo. Sort by prefsize-minsize, try to add first to all, then next.
//      If can't add to all => add to ones that will get to prefsize (thus, not equally).
//      If still have leftovers => add to all.
//
// Add geometry() (inside parent!) and layout(Recti) to Box => you have layout engine.
// Yes it mixes up logic, data and output. But, whatever.
//
// Then, scroll boxes.
//
// VBOX
//   Scroll
//   HBOX
//     Button
//     Box
//     Scroller
//     Box
//     Button
//
//
// OK, now, Widgets.
//
// Widget : PlatformEventFilter
//   Layout layout() - not inherited, data points back into Widget.
//
// EventPropagation:
// MouseEvent:
//   - do we have a mouse grabber?
//     - yes? send to it.
//       - maybe release mouse
//     - no? go through layout hierarchy all the way down.
//       - filters event? good, we're done.
//         - grab mouse if it was a mouse press.
//       - otherwise go to parent & repeat. data() gives widget.
//
// KeyboardEvent:
//   - do we have a focus widget?
//     - yes? send to it.
//     - no? ignore.
//
// Gui::setFocusWidget(Widget*)
// Gui::setMouseGrabber(Widget*)
//
// Button: Widget
// onPressed = []
//
// Label: Widget <= all font shenanigans are here.
//
// Stretch for stretchable UI elements.
//
// Portrait:
// onClick = []
// onDoubleClick = []
//
// VScrollArea
//

// For the images / textures caching.
// 1. Drop all the colorkey / xyz bullshit, move transparency keys and the need to interpolate to jsons.
// 2. weak_ptr<Texture> in AssetsManager
// 3. shared_ptr<Texture> in AssetsManager for shit that's always needed.
// 4. Texture has rgba() and renderId() (optional). indexed() is not needed there's only one usage for it - in cycling enchantment images. These should just be atlassed & cached.
//
// shared_ptrs are stored in sprite frame tables, in renderer, maybe somewhere else. For now just store them in AssetManager, and add gc() method there.
//
// AssetsManager has Source: SOURCE_ICONS, SOURCE_SPRITES, SOURCE_XYZ, SOURCE_GENERATED
//
// Also we need to generate EVERYTHING on startup. No lazy generations plz, no name parsing and fake generating file systems.
//
// Step 1 - atlassing the animations.
// Step 2 - dropping colorkey hardcode.
// Step 3 - redoing assetmanager. Loose textures belong to GUI classes.

/**
 * Some notes on markup characters supported by `GUIFont` functions.
 *
 * - `\n` is a regular line feed.
 * - `\tXXX`, where `XXX` is decimal offset in pixels. Moves the caret to a position offset by `XXX` to the right from
 *   the start of the line. This is how aligned tables are implemented.
 * - `\rXXX`, where `XXX` is a decimal offset in pixels. Moves the caret so that the rest of the line is
 *   right-justified, with the given offset from the right window border. This is used e.g. in the create party screen.
 * - `\fXXXXX`, where `XXXXX` is a decimal color code for 16-bit color to use for the text that follows.
 *
 * @see Color::fromC16
 */
class GUIFont {
 public:
    GUIFont();
    ~GUIFont();

    static std::unique_ptr<GUIFont> LoadFont(std::string_view pFontFile);

    void CreateFontTex();
    void ReleaseFontTex();

    bool IsCharValid(unsigned char c) const;
    int GetHeight() const;

    int AlignText_Center(int width, std::string_view str);

    int GetLineWidth(std::string_view str);

    int CalcTextHeight(std::string_view str, int width, int x_offset, bool return_on_carriage = false);

    std::string GetPageTop(std::string_view pInString, GUIWindow *pWindow,
                      unsigned int uX, int a5);

    /**
     * Draws a single line of text.
     *
     * @param text                          Input line of text.
     * @param color                         Color that the text should be started to be drawn at - this allows feeding
     *                                      in the color returned from the previous call to maintain correct color when
     *                                      it's split onto a new line.
     * @param defaultColor                  The color that the text should return to on hitting a default color tag.
     * @param position                      Position to draw the text line to.
     * @param max_len_pix                   The maximum allowed width for this line of text.
     * 
     * @return                              Color that was used to draw text at the end of the line.
     */
    Color DrawTextLine(std::string_view text, Color color, Color defaultColor, Pointi position, int max_len_pix);
    void DrawText(GUIWindow *window, Pointi position, Color color, std::string_view text, int maxHeight, Color shadowColor);
    int DrawTextInRect(GUIWindow *window, Pointi position,
                       Color color, std::string_view text, int rect_width,
                       int reverse_text);

    std::string FitTextInAWindow(std::string_view inString, int width, int uX, bool return_on_carriage = false);

    // TODO: these should take std::string_view
    void DrawCreditsEntry(GUIFont *pSecondFont, int uFrameX, int uFrameY,
                          unsigned int w, unsigned int h, Color firstColor,
                          Color secondColor, Color shadowColor, std::string_view pString,
                          GraphicsImage *image);
    int GetStringHeight2(GUIFont *secondFont, std::string_view text_str,
                         GUIWindow *pWindow, int startX, int a6);

    GraphicsImage *fonttex = nullptr;
    GraphicsImage *fontshadow = nullptr;

 private:
    std::string FitTwoFontStringINWindow(std::string_view inString, GUIFont *pFontSecond,
                                    GUIWindow *pWindow, int startPixlOff,
                                    bool return_on_carriage = false);
    void DrawTextLineToBuff(Color color, Color shadowColor, Color *uX_buff_pos,
                            std::string_view text, int line_width);

 private:
    LodFont _font;
};

void ReloadFonts();

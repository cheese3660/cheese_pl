//
// Created by Lexi Allen on 9/18/2022.
//

#ifndef CHEESE_VTERM_H
#define CHEESE_VTERM_H
#include <cstdint>
#include <string>
namespace cheese::io {
    extern bool useHyperLinks;
    enum class Color : std::uint8_t {
        DarkGray = 90,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        Black = 30,
        DarkRed,
        DarkGreen,
        DarkYellow,
        DarkBlue,
        DarkMagenta,
        DarkCyan,
        Gray
    };
    enum class Rendition : std::uint8_t {
        Reset = 0,
        Bold = 1,
        Faint = 2,
        Italic = 3,
        Underline = 4,
        Blink = 5,
        BlinkFast = 6,
        Invert = 7,
        Conceal = 8,
        Strike = 9,
        Font0 = 10,
        Font1 = 11,
        Font2 = 12,
        Font3 = 13,
        Font4 = 14,
        Font5 = 15,
        Font6 = 16,
        Font7 = 17,
        Font8 = 18,
        Font9 = 19,
        BlackletterFont = 20,
        DoubleUnderline = 21,
        NormalIntensity = 22,
        NotItalicNorBlackletter = 23,
        NotUnderlined = 24,
        NotBlinking = 25,
        ProportionalSpacing = 26,
        NotInverted = 27,
        Reveal = 28,
        NotCrossedOut = 29,
        DefaultForeground = 39,
        DefaultBackground = 49,
        DisableProportionalSpacing = 50,
        Framed = 51,
        Encircled = 52,
        Overlined = 53,
        NotFramedOrEncircled = 54,
        NotOverlined = 55,
        DefaultUnderlineColor = 59,
        IdeogramUnderlineRightLine = 60,
        IdeogramDoubleUnderlineRightLine = 61,
        IdeogramOverlineLeftLine = 62,
        IdeogramDoubleOverlineLeftLine =63,
        IdeogramStress = 64,
        NoIdeogram = 65,
        Superscript = 73,
        Subscript = 74,
        NoSuperSubScript = 75
    };
    constexpr char ESCAPE = 0x1B;
    void setForegroundColor(Color foreground);
    void setBackgroundColor(Color background);
    void setForegroundColor(std::uint8_t pal);
    void setForegroundColor(std::uint8_t r, std::uint8_t g, std::uint8_t b);
    void setBackgroundColor(std::uint8_t pal);
    void setBackgroundColor(std::uint8_t r, std::uint8_t g, std::uint8_t b);
    void setUnderlineColor(std::uint8_t r, std::uint8_t g, std::uint8_t b);
    void selectGraphicRendition(Rendition rend);
    void hyperlink(const char* link, const char* alt_text);
    void hyperlink(std::string link, std::string alt_text);
    void reset();
}

#endif //CHEESE_VTERM_H

//
// Created by Lexi Allen on 9/18/2022.
//
#include "vterm.h"
#include <iostream>
#include <map>
#include <format>

namespace cheese::io {
    bool useHyperLinks = false;

    inline void renditionBackend(std::uint8_t rendition) {
        std::cout << ESCAPE << '[' << static_cast<uint16_t>(rendition) << 'm';
    }

    void setForegroundColor(Color color) {
        renditionBackend(static_cast<uint8_t>(color));
    }

    void setForegroundColor(std::uint8_t color) {
        std::cout << ESCAPE << "[38;5;" << static_cast<std::uint16_t>(color) << 'm';
    }

    void setForegroundColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        std::cout << ESCAPE << "[38;2;" << static_cast<std::uint16_t>(r) << ';' << static_cast<std::uint16_t>(g) << ';'
                  << static_cast<uint16_t>(b) << 'm';
    }

    void setBackgroundColor(Color color) {
        renditionBackend(static_cast<uint8_t>(color) + 10);
    }

    void setBackgroundColor(std::uint8_t color) {
        std::cout << ESCAPE << "[48;5;" << static_cast<std::uint16_t>(color) << 'm';
    }

    void setBackgroundColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        std::cout << ESCAPE << "[48;2;" << static_cast<std::uint16_t>(r) << ';' << static_cast<std::uint16_t>(g) << ';'
                  << static_cast<uint16_t>(b) << 'm';
    }

    void setUnderlineColor(std::uint8_t color) {
        std::cout << ESCAPE << "[58;5;" << static_cast<std::uint16_t>(color) << 'm';
    }

    void setUnderlineColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        std::cout << ESCAPE << "[58;2;" << static_cast<std::uint16_t>(r) << ';' << static_cast<std::uint16_t>(g) << ';'
                  << static_cast<uint16_t>(b) << 'm';
    }

    void selectGraphicRendition(Rendition rendition) {
        renditionBackend(static_cast<uint8_t>(rendition));
    }

    void hyperlink(const char *link, const char *alt_text) {
        if (useHyperLinks) {
            std::cout << ESCAPE << "]8;;" << link << '\\' << ESCAPE << alt_text << ESCAPE << "]8;;\\";
        } else {
            std::cout << alt_text << " (" << link << ')';
        }
    }

    void hyperlink(std::string link, std::string alt_text) {
        hyperlink(link.c_str(), alt_text.c_str());
    }

    void reset() {
        setForegroundColor(Color::Gray);
        setBackgroundColor(Color::Black);
    }
}
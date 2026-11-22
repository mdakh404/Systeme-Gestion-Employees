#ifndef GESTION_ABSENCES_UTF8_H
#define GESTION_ABSENCES_UTF8_H

#include <string_view>

namespace utf8 {

inline bool lirePoint(std::string_view texte, std::size_t& position, char32_t& point) {
    const auto premier = static_cast<unsigned char>(texte[position]);
    if (premier < 0x80) {
        point = premier;
        ++position;
        return true;
    }
    if (premier < 0xC2 || premier > 0xF4) {
        return false;
    }
    const std::size_t taille = premier < 0xE0 ? 2 : premier < 0xF0 ? 3 : 4;
    if (texte.size() - position < taille) {
        return false;
    }
    point = premier & (0x7F >> taille);
    for (std::size_t i = 1; i < taille; ++i) {
        const auto octet = static_cast<unsigned char>(texte[position + i]);
        if ((octet & 0xC0) != 0x80) {
            return false;
        }
        point = (point << 6) | (octet & 0x3F);
    }
    if ((taille == 3 && point < 0x800) || (taille == 4 && point < 0x10000)
        || (point >= 0xD800 && point <= 0xDFFF) || point > 0x10FFFF) {
        return false;
    }
    position += taille;
    return true;
}

inline bool estBlanc(char32_t point) {
    return (point >= 0x09 && point <= 0x0D) || point == 0x20 || point == 0x85
           || point == 0xA0 || point == 0x1680 || (point >= 0x2000 && point <= 0x200A)
           || point == 0x2028 || point == 0x2029 || point == 0x202F || point == 0x205F
           || point == 0x3000;
}

} // namespace utf8

#endif

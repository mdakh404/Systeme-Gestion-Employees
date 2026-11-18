#include "Validation.h"
#include <cctype>

namespace Validation {

bool sansSeparateur(const std::string& s) {
    for (char c : s)
        if (c == ';' || c == '|' || c == '\n' || c == '\r') return false;
    return true;
}

bool nomValide(const std::string& s) {
    if (s.size() < 2 || s.size() > 50) return false;
    for (char ch : s) {
        unsigned char c = static_cast<unsigned char>(ch);
        // c >= 128 : octets UTF-8 (lettres accentuees)
        if (!(std::isalpha(c) || c >= 128 || c == ' ' || c == '-' || c == '\'')) return false;
    }
    return true;
}

bool nomDepartementValide(const std::string& s) {
    if (s.size() < 2 || s.size() > 50) return false;
    for (char ch : s) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (!(std::isalnum(c) || c >= 128 || c == ' ' || c == '-' || c == '\'' || c == '&')) return false;
    }
    return true;
}

bool emailValide(const std::string& s) {
    if (s.size() < 5 || s.size() > 100 || !sansSeparateur(s)) return false;
    size_t at = s.find('@');
    if (at == std::string::npos || at == 0 || s.find('@', at + 1) != std::string::npos) return false;
    for (char c : s) if (std::isspace(static_cast<unsigned char>(c))) return false;
    size_t dot = s.rfind('.');
    return dot != std::string::npos && dot > at + 1 && dot + 1 < s.size();
}

bool telephoneValide(const std::string& s) {
    size_t i = (!s.empty() && s[0] == '+') ? 1 : 0;
    size_t chiffres = s.size() - i;
    if (chiffres < 8 || chiffres > 15) return false;
    for (; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

bool cinValide(const std::string& s) {
    if (s.size() < 5 || s.size() > 12) return false;
    for (char c : s)
        if (!std::isalnum(static_cast<unsigned char>(c))) return false;
    return true;
}

} // namespace Validation

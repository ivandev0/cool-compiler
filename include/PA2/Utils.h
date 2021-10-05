#pragma once
#include <algorithm>
#include <cctype>
#include <string>

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isAlphaOdDigitOrUnderscore(char c) {
    return isAlpha(c) || isDigit(c) || c == '_';
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\f' || c == '\r' || c == '\t' || c == '\v' || c == '\n';
}

std::string toLowerCase(const std::string& str) {
    auto copy = std::string(str);
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c){ return std::tolower(c); });
    return copy;
}

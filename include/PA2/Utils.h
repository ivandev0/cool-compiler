#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include<iomanip>

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

std::string charToStringRepresentation(char c) {
    if (c == '\\') return "\\\\";
    if ((int) c >= 32) return std::string(1, c);
    switch (c) {
        case '\n': return "\\n";
        case '\f': return "\\f";
        case '\t': return "\\t";
        case '\b': return "\\b";
    }
    std::stringstream str;
    str << '\\' << std::oct << std::setw(3) << std::setfill('0') << (int) c;
    return str.str();
}


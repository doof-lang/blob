#pragma once

#include "doof_runtime.hpp"
#include "types.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace doof_blob {

[[noreturn]] inline void panicArgument(const std::string& message) {
    doof::panic("blob " + message);
}

inline bool hostIsLittleEndian() {
    const uint16_t marker = 1;
    return *reinterpret_cast<const uint8_t*>(&marker) == 1;
}

inline size_t checkedSize(int64_t value, const char* context) {
    if (value < 0) {
        panicArgument(std::string(context) + " must not be negative");
    }

    if (static_cast<uint64_t>(value) > static_cast<uint64_t>(std::numeric_limits<size_t>::max())) {
        panicArgument(std::string(context) + " is too large for this runtime");
    }

    return static_cast<size_t>(value);
}

inline int64_t checkedAdvance(int64_t start, size_t width, const char* context) {
    if (start < 0) {
        panicArgument(std::string(context) + " must not be negative");
    }

    constexpr uint64_t maxLong = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    if (static_cast<uint64_t>(start) > maxLong - static_cast<uint64_t>(width)) {
        panicArgument(std::string(context) + " exceeds the supported blob size");
    }

    return start + static_cast<int64_t>(width);
}

inline int64_t alignedPosition(int64_t position, int64_t width) {
    if (width <= 0) {
        panicArgument("alignment width must be positive");
    }

    const uint64_t unsignedPosition = static_cast<uint64_t>(position);
    const uint64_t unsignedWidth = static_cast<uint64_t>(width);
    const uint64_t remainder = unsignedPosition % unsignedWidth;
    if (remainder == 0) {
        return position;
    }

    const uint64_t delta = unsignedWidth - remainder;
    constexpr uint64_t maxLong = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    if (unsignedPosition > maxLong - delta) {
        panicArgument("aligned position exceeds the supported blob size");
    }

    return static_cast<int64_t>(unsignedPosition + delta);
}

inline bool checkedIntSize(size_t value) {
    return value <= static_cast<size_t>(std::numeric_limits<int32_t>::max());
}

template <typename T>
inline T checkedIntegerRange(int64_t value, const char* operation) {
    if (value < static_cast<int64_t>(std::numeric_limits<T>::min()) ||
        value > static_cast<int64_t>(std::numeric_limits<T>::max())) {
        panicArgument(std::string(operation) + " value is outside the supported range");
    }

    return static_cast<T>(value);
}

template <typename T>
inline T checkedUnsignedIntegerRange(int64_t value, const char* operation) {
    if (value < 0 || static_cast<uint64_t>(value) > static_cast<uint64_t>(std::numeric_limits<T>::max())) {
        panicArgument(std::string(operation) + " value is outside the supported range");
    }

    return static_cast<T>(value);
}

inline doof::Result<int32_t, EncodingError> encodingFailure(EncodingError error) {
    return doof::Failure<EncodingError>{error};
}

inline doof::Result<std::string, EncodingError> decodingFailure(EncodingError error) {
    return doof::Failure<EncodingError>{error};
}

inline const std::array<char32_t, 128>& windows1252Table() {
    static const std::array<char32_t, 128> table {
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
        0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
        0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
        0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
        0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
        0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
        0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
        0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
        0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
        0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
        0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
        0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
        0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
        0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,
    };
    return table;
}

inline const std::array<char32_t, 128>& cp437Table() {
    static const std::array<char32_t, 128> table {
        0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
        0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
        0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
        0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
        0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
        0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
        0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
        0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
        0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
        0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
        0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
        0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
        0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
        0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
        0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
        0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
    };
    return table;
}

inline void appendUtf8(std::string& output, char32_t codePoint) {
    if (codePoint <= 0x7F) {
        output.push_back(static_cast<char>(codePoint));
    } else if (codePoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else if (codePoint <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }
}

inline void appendUtf8Bytes(std::vector<uint8_t>& output, char32_t codePoint) {
    std::string encoded;
    appendUtf8(encoded, codePoint);
    output.insert(output.end(), encoded.begin(), encoded.end());
}

inline bool readUtf8CodePoint(const std::string& input, size_t& index, char32_t& codePoint) {
    if (index >= input.size()) {
        return false;
    }

    const uint8_t first = static_cast<uint8_t>(input[index]);
    if (first <= 0x7F) {
        codePoint = first;
        index += 1;
        return true;
    }

    size_t width = 0;
    char32_t value = 0;
    char32_t minValue = 0;
    if (first >= 0xC2 && first <= 0xDF) {
        width = 2;
        value = first & 0x1F;
        minValue = 0x80;
    } else if (first >= 0xE0 && first <= 0xEF) {
        width = 3;
        value = first & 0x0F;
        minValue = 0x800;
    } else if (first >= 0xF0 && first <= 0xF4) {
        width = 4;
        value = first & 0x07;
        minValue = 0x10000;
    } else {
        return false;
    }

    if (index + width > input.size()) {
        return false;
    }

    for (size_t offset = 1; offset < width; offset++) {
        const uint8_t next = static_cast<uint8_t>(input[index + offset]);
        if ((next & 0xC0) != 0x80) {
            return false;
        }
        value = (value << 6) | (next & 0x3F);
    }

    if (value < minValue || value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF)) {
        return false;
    }

    codePoint = value;
    index += width;
    return true;
}

inline std::optional<uint8_t> encodeWindows1252CodePoint(char32_t codePoint) {
    if (codePoint <= 0x7F || (codePoint >= 0xA0 && codePoint <= 0xFF)) {
        return static_cast<uint8_t>(codePoint);
    }

    const auto& table = windows1252Table();
    for (size_t index = 0; index < 32; index++) {
        if (table[index] == codePoint) {
            return static_cast<uint8_t>(0x80 + index);
        }
    }
    return std::nullopt;
}

inline std::optional<uint8_t> encodeCp437CodePoint(char32_t codePoint) {
    if (codePoint <= 0x7F) {
        return static_cast<uint8_t>(codePoint);
    }

    const auto& table = cp437Table();
    for (size_t index = 0; index < table.size(); index++) {
        if (table[index] == codePoint) {
            return static_cast<uint8_t>(0x80 + index);
        }
    }
    return std::nullopt;
}

inline bool readUtf8CodePointLossy(const std::string& input, size_t& index, char32_t& codePoint) {
    const size_t start = index;
    if (readUtf8CodePoint(input, index, codePoint)) {
        return true;
    }

    index = std::min(start + 1, input.size());
    codePoint = 0xFFFD;
    return false;
}

inline doof::Result<std::vector<uint8_t>, EncodingError> encodeTextBytes(const std::string& value, TextEncoding encoding) {
    std::vector<uint8_t> output;
    if (encoding == TextEncoding::Utf8) {
        if (!checkedIntSize(value.size())) {
            return doof::Failure<EncodingError>{EncodingError::OutputTooLarge};
        }

        size_t index = 0;
        char32_t ignored = 0;
        while (index < value.size()) {
            if (!readUtf8CodePoint(value, index, ignored)) {
                return doof::Failure<EncodingError>{EncodingError::InvalidData};
            }
        }

        output.assign(value.begin(), value.end());
        return doof::Success<std::vector<uint8_t>>{output};
    }

    size_t index = 0;
    while (index < value.size()) {
        char32_t codePoint = 0;
        if (!readUtf8CodePoint(value, index, codePoint)) {
            return doof::Failure<EncodingError>{EncodingError::InvalidData};
        }

        if (encoding == TextEncoding::Utf16LE || encoding == TextEncoding::Utf16BE) {
            auto appendUnit = [&](uint16_t unit) {
                if (encoding == TextEncoding::Utf16LE) {
                    output.push_back(static_cast<uint8_t>(unit & 0xFF));
                    output.push_back(static_cast<uint8_t>(unit >> 8));
                } else {
                    output.push_back(static_cast<uint8_t>(unit >> 8));
                    output.push_back(static_cast<uint8_t>(unit & 0xFF));
                }
            };

            if (codePoint <= 0xFFFF) {
                appendUnit(static_cast<uint16_t>(codePoint));
            } else {
                const char32_t scalar = codePoint - 0x10000;
                appendUnit(static_cast<uint16_t>(0xD800 + (scalar >> 10)));
                appendUnit(static_cast<uint16_t>(0xDC00 + (scalar & 0x3FF)));
            }
            continue;
        }

        std::optional<uint8_t> encoded;
        if (encoding == TextEncoding::Ascii) {
            if (codePoint <= 0x7F) {
                encoded = static_cast<uint8_t>(codePoint);
            }
        } else if (encoding == TextEncoding::Latin1) {
            if (codePoint <= 0xFF) {
                encoded = static_cast<uint8_t>(codePoint);
            }
        } else if (encoding == TextEncoding::Windows1252) {
            encoded = encodeWindows1252CodePoint(codePoint);
        } else if (encoding == TextEncoding::CP437) {
            encoded = encodeCp437CodePoint(codePoint);
        }

        if (!encoded.has_value()) {
            return doof::Failure<EncodingError>{EncodingError::UnrepresentableCharacter};
        }
        output.push_back(encoded.value());
    }

    if (!checkedIntSize(output.size())) {
        return doof::Failure<EncodingError>{EncodingError::OutputTooLarge};
    }
    return doof::Success<std::vector<uint8_t>>{output};
}

inline std::vector<uint8_t> encodeTextBytesLossy(const std::string& value, TextEncoding encoding) {
    std::vector<uint8_t> output;
    if (encoding == TextEncoding::Utf8) {
        size_t index = 0;
        while (index < value.size()) {
            char32_t codePoint = 0;
            const bool valid = readUtf8CodePointLossy(value, index, codePoint);
            if (valid) {
                appendUtf8Bytes(output, codePoint);
            } else {
                output.push_back(static_cast<uint8_t>('?'));
            }
        }
        return output;
    }

    size_t index = 0;
    while (index < value.size()) {
        char32_t codePoint = 0;
        const bool valid = readUtf8CodePointLossy(value, index, codePoint);
        if (!valid) {
            codePoint = '?';
        }

        if (encoding == TextEncoding::Utf16LE || encoding == TextEncoding::Utf16BE) {
            auto appendUnit = [&](uint16_t unit) {
                if (encoding == TextEncoding::Utf16LE) {
                    output.push_back(static_cast<uint8_t>(unit & 0xFF));
                    output.push_back(static_cast<uint8_t>(unit >> 8));
                } else {
                    output.push_back(static_cast<uint8_t>(unit >> 8));
                    output.push_back(static_cast<uint8_t>(unit & 0xFF));
                }
            };

            if (codePoint <= 0xFFFF) {
                appendUnit(static_cast<uint16_t>(codePoint));
            } else {
                const char32_t scalar = codePoint - 0x10000;
                appendUnit(static_cast<uint16_t>(0xD800 + (scalar >> 10)));
                appendUnit(static_cast<uint16_t>(0xDC00 + (scalar & 0x3FF)));
            }
            continue;
        }

        std::optional<uint8_t> encoded;
        if (encoding == TextEncoding::Ascii) {
            if (codePoint <= 0x7F) {
                encoded = static_cast<uint8_t>(codePoint);
            }
        } else if (encoding == TextEncoding::Latin1) {
            if (codePoint <= 0xFF) {
                encoded = static_cast<uint8_t>(codePoint);
            }
        } else if (encoding == TextEncoding::Windows1252) {
            encoded = encodeWindows1252CodePoint(codePoint);
        } else if (encoding == TextEncoding::CP437) {
            encoded = encodeCp437CodePoint(codePoint);
        }

        output.push_back(encoded.value_or(static_cast<uint8_t>('?')));
    }

    return output;
}

inline uint16_t readUtf16Unit(const std::vector<uint8_t>& data, size_t index, TextEncoding encoding) {
    if (encoding == TextEncoding::Utf16LE) {
        return static_cast<uint16_t>(data[index]) | (static_cast<uint16_t>(data[index + 1]) << 8);
    }
    return (static_cast<uint16_t>(data[index]) << 8) | static_cast<uint16_t>(data[index + 1]);
}

inline doof::Result<std::string, EncodingError> decodeTextBytes(
    const std::vector<uint8_t>& data,
    TextEncoding encoding
) {
    std::string output;
    if (encoding == TextEncoding::Utf8) {
        output.assign(data.begin(), data.end());
        size_t index = 0;
        char32_t ignored = 0;
        while (index < output.size()) {
            if (!readUtf8CodePoint(output, index, ignored)) {
                return decodingFailure(EncodingError::InvalidData);
            }
        }
        return doof::Success<std::string>{output};
    }

    if (encoding == TextEncoding::Utf16LE || encoding == TextEncoding::Utf16BE) {
        if ((data.size() % 2) != 0) {
            return decodingFailure(EncodingError::InvalidData);
        }

        for (size_t index = 0; index < data.size(); index += 2) {
            const uint16_t unit = readUtf16Unit(data, index, encoding);
            if (unit >= 0xD800 && unit <= 0xDBFF) {
                if (index + 3 >= data.size()) {
                    return decodingFailure(EncodingError::InvalidData);
                }

                const uint16_t low = readUtf16Unit(data, index + 2, encoding);
                if (low < 0xDC00 || low > 0xDFFF) {
                    return decodingFailure(EncodingError::InvalidData);
                }

                const char32_t codePoint = 0x10000 + (((unit - 0xD800) << 10) | (low - 0xDC00));
                appendUtf8(output, codePoint);
                index += 2;
            } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
                return decodingFailure(EncodingError::InvalidData);
            } else {
                appendUtf8(output, unit);
            }
        }
        return doof::Success<std::string>{output};
    }

    const auto& win1252 = windows1252Table();
    const auto& cp437 = cp437Table();
    for (uint8_t byte : data) {
        char32_t codePoint = byte;
        if (encoding == TextEncoding::Ascii) {
            if (byte > 0x7F) {
                return decodingFailure(EncodingError::InvalidData);
            }
        } else if (encoding == TextEncoding::Windows1252 && byte >= 0x80) {
            codePoint = win1252[byte - 0x80];
        } else if (encoding == TextEncoding::CP437 && byte >= 0x80) {
            codePoint = cp437[byte - 0x80];
        }
        appendUtf8(output, codePoint);
    }

    return doof::Success<std::string>{output};
}

inline std::string decodeTextBytesLossy(
    const std::vector<uint8_t>& data,
    TextEncoding encoding
) {
    std::string output;
    if (encoding == TextEncoding::Utf8) {
        std::string input(data.begin(), data.end());
        size_t index = 0;
        while (index < input.size()) {
            char32_t codePoint = 0;
            readUtf8CodePointLossy(input, index, codePoint);
            appendUtf8(output, codePoint);
        }
        return output;
    }

    if (encoding == TextEncoding::Utf16LE || encoding == TextEncoding::Utf16BE) {
        size_t index = 0;
        while (index + 1 < data.size()) {
            const uint16_t unit = readUtf16Unit(data, index, encoding);
            if (unit >= 0xD800 && unit <= 0xDBFF) {
                if (index + 3 < data.size()) {
                    const uint16_t low = readUtf16Unit(data, index + 2, encoding);
                    if (low >= 0xDC00 && low <= 0xDFFF) {
                        const char32_t codePoint = 0x10000 + (((unit - 0xD800) << 10) | (low - 0xDC00));
                        appendUtf8(output, codePoint);
                        index += 4;
                        continue;
                    }
                }
                appendUtf8(output, 0xFFFD);
            } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
                appendUtf8(output, 0xFFFD);
            } else {
                appendUtf8(output, unit);
            }
            index += 2;
        }

        if (index < data.size()) {
            appendUtf8(output, 0xFFFD);
        }
        return output;
    }

    const auto& win1252 = windows1252Table();
    const auto& cp437 = cp437Table();
    for (uint8_t byte : data) {
        char32_t codePoint = byte;
        if (encoding == TextEncoding::Ascii) {
            if (byte > 0x7F) {
                codePoint = 0xFFFD;
            }
        } else if (encoding == TextEncoding::Windows1252 && byte >= 0x80) {
            codePoint = win1252[byte - 0x80];
        } else if (encoding == TextEncoding::CP437 && byte >= 0x80) {
            codePoint = cp437[byte - 0x80];
        }
        appendUtf8(output, codePoint);
    }

    return output;
}

template <typename T>
inline T byteSwap(T value) {
    std::array<uint8_t, sizeof(T)> bytes {};
    std::memcpy(bytes.data(), &value, sizeof(T));
    std::reverse(bytes.begin(), bytes.end());

    T swapped {};
    std::memcpy(&swapped, bytes.data(), sizeof(T));
    return swapped;
}

template <typename T>
inline T convertEndian(T value, Endian endianness) {
    if (sizeof(T) == 1) {
        return value;
    }

    const bool wantsLittleEndian = endianness == Endian::LittleEndian;
    if (hostIsLittleEndian() == wantsLittleEndian) {
        return value;
    }

    return byteSwap(value);
}

[[noreturn]] inline void panicReadOutOfBounds(const char* operation, int64_t position, size_t width, size_t length) {
    doof::panic(
        std::string("blob ") + operation + " would read beyond the end of the blob at position " +
        std::to_string(position) + " (need " + std::to_string(width) + " bytes, length " + std::to_string(length) + ")"
    );
}

class NativeBlobBuilder {
public:
    static std::shared_ptr<NativeBlobBuilder> constructor(int64_t size, Endian endianness) {
        auto builder = std::shared_ptr<NativeBlobBuilder>(new NativeBlobBuilder(endianness));
        builder->buffer_.reserve(checkedSize(size, "builder size"));
        return builder;
    }

    int64_t getPosition() const {
        return position_;
    }

    void setPosition(int64_t position) {
        ensureSize(position);
        position_ = position;
    }

    int64_t length() const {
        return static_cast<int64_t>(buffer_.size());
    }

    void writeZeroes(int64_t length) {
        const size_t width = checkedSize(length, "writeZeroes length");
        const int64_t endPosition = checkedAdvance(position_, width, "write position");
        ensureSize(endPosition);
        if (width > 0) {
            std::fill(
                buffer_.begin() + checkedSize(position_, "position"),
                buffer_.begin() + checkedSize(endPosition, "position"),
                0
            );
        }
        position_ = endPosition;
    }

    void align(int64_t width) {
        setPosition(alignedPosition(position_, width));
    }

    void writeByte(uint8_t value) {
        writeRaw(&value, sizeof(value));
    }

    void writeSignedByte(int32_t value) {
        writeScalar(checkedIntegerRange<int8_t>(value, "writeSignedByte"));
    }

    void writeBool(bool value) {
        const uint8_t raw = value ? 1 : 0;
        writeRaw(&raw, sizeof(raw));
    }

    void writeShort(int32_t value) {
        writeScalar(checkedIntegerRange<int16_t>(value, "writeShort"));
    }

    void writeUnsignedShort(int32_t value) {
        writeScalar(checkedUnsignedIntegerRange<uint16_t>(value, "writeUnsignedShort"));
    }

    void writeInt(int32_t value) {
        writeScalar(value);
    }

    void writeUnsignedInt(int64_t value) {
        writeScalar(checkedUnsignedIntegerRange<uint32_t>(value, "writeUnsignedInt"));
    }

    void writeLong(int64_t value) {
        writeScalar(value);
    }

    void writeFloat(float value) {
        const float encoded = convertEndian(value, endianness_);
        writeRaw(reinterpret_cast<const uint8_t*>(&encoded), sizeof(encoded));
    }

    void writeDouble(double value) {
        const double encoded = convertEndian(value, endianness_);
        writeRaw(reinterpret_cast<const uint8_t*>(&encoded), sizeof(encoded));
    }

    void writeBytes(const std::shared_ptr<std::vector<uint8_t>>& value) {
        if (!value || value->empty()) {
            return;
        }

        writeRaw(value->data(), value->size());
    }

    void writeString(const std::string& value) {
        if (value.empty()) {
            return;
        }

        writeRaw(reinterpret_cast<const uint8_t*>(value.data()), value.size());
    }

    doof::Result<int32_t, EncodingError> writeText(const std::string& value, TextEncoding encoding) {
        auto encoded = encodeTextBytes(value, encoding);
        if (is_failure(encoded)) {
            return encodingFailure(failure_error(encoded));
        }

        const auto& bytes = success_value(encoded);
        if (!checkedIntSize(bytes.size())) {
            return encodingFailure(EncodingError::OutputTooLarge);
        }
        if (!bytes.empty()) {
            writeRaw(bytes.data(), bytes.size());
        }
        return doof::Success<int32_t>{static_cast<int32_t>(bytes.size())};
    }

    int32_t writeTextLossy(const std::string& value, TextEncoding encoding) {
        const auto bytes = encodeTextBytesLossy(value, encoding);
        if (!checkedIntSize(bytes.size())) {
            panicArgument("writeTextLossy output is too large to report as an int");
        }
        if (!bytes.empty()) {
            writeRaw(bytes.data(), bytes.size());
        }
        return static_cast<int32_t>(bytes.size());
    }

    std::shared_ptr<std::vector<uint8_t>> build() {
        auto result = std::make_shared<std::vector<uint8_t>>(buffer_);
        std::vector<uint8_t>().swap(buffer_);
        position_ = 0;
        return result;
    }

private:
    explicit NativeBlobBuilder(Endian endianness)
        : position_(0), endianness_(endianness) {}

    void ensureSize(int64_t size) {
        const size_t normalizedSize = checkedSize(size, "position");
        if (buffer_.size() < normalizedSize) {
            buffer_.resize(normalizedSize, 0);
        }
    }

    void writeRaw(const uint8_t* data, size_t width) {
        const int64_t endPosition = checkedAdvance(position_, width, "write position");
        ensureSize(endPosition);
        if (width > 0) {
            std::memcpy(buffer_.data() + checkedSize(position_, "position"), data, width);
        }
        position_ = endPosition;
    }

    template <typename T>
    void writeScalar(T value) {
        const T encoded = convertEndian(value, endianness_);
        writeRaw(reinterpret_cast<const uint8_t*>(&encoded), sizeof(encoded));
    }

    std::vector<uint8_t> buffer_;
    int64_t position_;
    Endian endianness_;
};

class NativeBlobReader {
public:
    static std::shared_ptr<NativeBlobReader> constructor(
        const std::shared_ptr<std::vector<uint8_t>>& data,
        Endian endianness
    ) {
        return std::shared_ptr<NativeBlobReader>(new NativeBlobReader(data, endianness));
    }

    std::shared_ptr<std::vector<uint8_t>> data;

    int64_t getPosition() const {
        return position_;
    }

    void setPosition(int64_t position) {
        if (position < 0 || static_cast<uint64_t>(position) > static_cast<uint64_t>(data->size())) {
            panicArgument(
                "reader position " + std::to_string(position) +
                " is outside the blob bounds (length " + std::to_string(data->size()) + ")"
            );
        }

        position_ = position;
    }

    int64_t length() const {
        return static_cast<int64_t>(data->size());
    }

    int64_t remaining() const {
        return static_cast<int64_t>(data->size()) - position_;
    }

    uint8_t peekByte() const {
        ensureReadable(sizeof(uint8_t), "peekByte");
        return (*data)[checkedSize(position_, "position")];
    }

    void skip(int64_t length) {
        const size_t width = checkedSize(length, "skip length");
        ensureReadable(width, "skip");
        position_ = checkedAdvance(position_, width, "read position");
    }

    void align(int64_t width) {
        const int64_t aligned = alignedPosition(position_, width);
        const size_t advance = checkedSize(aligned - position_, "alignment advance");
        ensureReadable(advance, "align");
        position_ = aligned;
    }

    uint8_t readByte() {
        ensureReadable(sizeof(uint8_t), "readByte");
        const uint8_t value = (*data)[checkedSize(position_, "position")];
        position_ = checkedAdvance(position_, sizeof(uint8_t), "read position");
        return value;
    }

    bool readBool() {
        return readByte() != 0;
    }

    int32_t readSignedByte() {
        return static_cast<int32_t>(readScalar<int8_t>("readSignedByte"));
    }

    int32_t readShort() {
        return static_cast<int32_t>(readScalar<int16_t>("readShort"));
    }

    int32_t readUnsignedShort() {
        return static_cast<int32_t>(readScalar<uint16_t>("readUnsignedShort"));
    }

    int32_t readInt() {
        return readScalar<int32_t>("readInt");
    }

    int64_t readUnsignedInt() {
        return static_cast<int64_t>(readScalar<uint32_t>("readUnsignedInt"));
    }

    int64_t readLong() {
        return readScalar<int64_t>("readLong");
    }

    float readFloat() {
        return readScalar<float>("readFloat");
    }

    double readDouble() {
        return readScalar<double>("readDouble");
    }

    std::shared_ptr<std::vector<uint8_t>> readBytes(int64_t length) {
        const size_t width = checkedSize(length, "read length");
        ensureReadable(width, "readBytes");

        const size_t start = checkedSize(position_, "position");
        auto result = std::make_shared<std::vector<uint8_t>>(data->begin() + start, data->begin() + start + width);
        position_ = checkedAdvance(position_, width, "read position");
        return result;
    }

    std::string readString(int64_t length) {
        const size_t width = checkedSize(length, "read length");
        ensureReadable(width, "readString");

        const size_t start = checkedSize(position_, "position");
        std::string value(reinterpret_cast<const char*>(data->data() + start), width);
        position_ = checkedAdvance(position_, width, "read position");
        return value;
    }

    doof::Result<std::string, EncodingError> readText(int64_t length, TextEncoding encoding) {
        const size_t width = checkedSize(length, "read length");
        ensureReadable(width, "readText");

        const size_t start = checkedSize(position_, "position");
        std::vector<uint8_t> bytes(data->begin() + start, data->begin() + start + width);
        auto decoded = decodeTextBytes(bytes, encoding);
        if (is_failure(decoded)) {
            return decoded;
        }

        position_ = checkedAdvance(position_, width, "read position");
        return decoded;
    }

    std::string readTextLossy(int64_t length, TextEncoding encoding) {
        const size_t width = checkedSize(length, "read length");
        ensureReadable(width, "readTextLossy");

        const size_t start = checkedSize(position_, "position");
        std::vector<uint8_t> bytes(data->begin() + start, data->begin() + start + width);
        auto decoded = decodeTextBytesLossy(bytes, encoding);
        position_ = checkedAdvance(position_, width, "read position");
        return decoded;
    }

    std::optional<int64_t> findNextAny(const std::shared_ptr<std::vector<uint8_t>>& candidates) const {
        if (!candidates || candidates->empty()) {
            return std::nullopt;
        }

        std::array<bool, 256> candidateSet {};
        for (uint8_t candidate : *candidates) {
            candidateSet[candidate] = true;
        }

        const size_t start = checkedSize(position_, "position");
        for (size_t index = start; index < data->size(); index++) {
            if (candidateSet[(*data)[index]]) {
                return static_cast<int64_t>(index);
            }
        }

        return std::nullopt;
    }

private:
    NativeBlobReader(const std::shared_ptr<std::vector<uint8_t>>& data, Endian endianness)
        : data(data ? data : std::make_shared<std::vector<uint8_t>>()), position_(0), endianness_(endianness) {}

    void ensureReadable(size_t width, const char* operation) const {
        const size_t start = checkedSize(position_, "position");
        if (start > data->size() || width > data->size() - start) {
            panicReadOutOfBounds(operation, position_, width, data->size());
        }
    }

    template <typename T>
    T readScalar(const char* operation) {
        ensureReadable(sizeof(T), operation);

        const size_t start = checkedSize(position_, "position");
        T encoded {};
        std::memcpy(&encoded, data->data() + start, sizeof(T));
        position_ = checkedAdvance(position_, sizeof(T), "read position");
        return convertEndian(encoded, endianness_);
    }

    int64_t position_;
    Endian endianness_;
};

} // namespace doof_blob

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
    static std::shared_ptr<NativeBlobBuilder> create(int64_t size, Endian endianness) {
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

    void writeByte(uint8_t value) {
        writeRaw(&value, sizeof(value));
    }

    void writeBool(bool value) {
        const uint8_t raw = value ? 1 : 0;
        writeRaw(&raw, sizeof(raw));
    }

    void writeInt(int32_t value) {
        const int32_t encoded = convertEndian(value, endianness_);
        writeRaw(reinterpret_cast<const uint8_t*>(&encoded), sizeof(encoded));
    }

    void writeLong(int64_t value) {
        const int64_t encoded = convertEndian(value, endianness_);
        writeRaw(reinterpret_cast<const uint8_t*>(&encoded), sizeof(encoded));
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

    std::vector<uint8_t> buffer_;
    int64_t position_;
    Endian endianness_;
};

class NativeBlobReader {
public:
    static std::shared_ptr<NativeBlobReader> create(
        const std::shared_ptr<std::vector<uint8_t>>& data,
        Endian endianness
    ) {
        return std::shared_ptr<NativeBlobReader>(new NativeBlobReader(data, endianness));
    }

    int64_t getPosition() const {
        return position_;
    }

    void setPosition(int64_t position) {
        if (position < 0 || static_cast<uint64_t>(position) > static_cast<uint64_t>(data_->size())) {
            panicArgument(
                "reader position " + std::to_string(position) +
                " is outside the blob bounds (length " + std::to_string(data_->size()) + ")"
            );
        }

        position_ = position;
    }

    int64_t length() const {
        return static_cast<int64_t>(data_->size());
    }

    int64_t remaining() const {
        return static_cast<int64_t>(data_->size()) - position_;
    }

    uint8_t readByte() {
        ensureReadable(sizeof(uint8_t), "readByte");
        const uint8_t value = (*data_)[checkedSize(position_, "position")];
        position_ = checkedAdvance(position_, sizeof(uint8_t), "read position");
        return value;
    }

    bool readBool() {
        return readByte() != 0;
    }

    int32_t readInt() {
        return readScalar<int32_t>("readInt");
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
        auto result = std::make_shared<std::vector<uint8_t>>(data_->begin() + start, data_->begin() + start + width);
        position_ = checkedAdvance(position_, width, "read position");
        return result;
    }

    std::string readString(int64_t length) {
        const size_t width = checkedSize(length, "read length");
        ensureReadable(width, "readString");

        const size_t start = checkedSize(position_, "position");
        std::string value(reinterpret_cast<const char*>(data_->data() + start), width);
        position_ = checkedAdvance(position_, width, "read position");
        return value;
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
        for (size_t index = start; index < data_->size(); index++) {
            if (candidateSet[(*data_)[index]]) {
                return static_cast<int64_t>(index);
            }
        }

        return std::nullopt;
    }

private:
    NativeBlobReader(const std::shared_ptr<std::vector<uint8_t>>& data, Endian endianness)
        : data_(data ? data : std::make_shared<std::vector<uint8_t>>()), position_(0), endianness_(endianness) {}

    void ensureReadable(size_t width, const char* operation) const {
        const size_t start = checkedSize(position_, "position");
        if (start > data_->size() || width > data_->size() - start) {
            panicReadOutOfBounds(operation, position_, width, data_->size());
        }
    }

    template <typename T>
    T readScalar(const char* operation) {
        ensureReadable(sizeof(T), operation);

        const size_t start = checkedSize(position_, "position");
        T encoded {};
        std::memcpy(&encoded, data_->data() + start, sizeof(T));
        position_ = checkedAdvance(position_, sizeof(T), "read position");
        return convertEndian(encoded, endianness_);
    }

    std::shared_ptr<std::vector<uint8_t>> data_;
    int64_t position_;
    Endian endianness_;
};

} // namespace doof_blob
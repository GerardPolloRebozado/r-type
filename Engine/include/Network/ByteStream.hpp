#ifndef ENGINE_NETWORK_BYTE_STREAM_HPP_
#define ENGINE_NETWORK_BYTE_STREAM_HPP_

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace Engine::Network {

/**
 * @brief High-performance binary buffer writer for network packet serialization.
 */
class ByteWriter {
  public:
    ByteWriter() = default;

    template <typename T> void Write(const T& val) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        const auto* ptr = reinterpret_cast<const uint8_t*>(&val);
        buffer_.insert(buffer_.end(), ptr, ptr + sizeof(T));
    }

    void WriteString(const std::string& str) {
        auto len = static_cast<uint16_t>(str.size());
        Write(len);
        buffer_.insert(buffer_.end(), str.begin(), str.end());
    }

    void WriteBytes(const uint8_t* data, size_t size) {
        if (data && size > 0) {
            buffer_.insert(buffer_.end(), data, data + size);
        }
    }

    [[nodiscard]] const std::vector<uint8_t>& GetData() const noexcept {
        return buffer_;
    }
    [[nodiscard]] const uint8_t* Data() const noexcept {
        return buffer_.data();
    }
    [[nodiscard]] size_t Size() const noexcept {
        return buffer_.size();
    }

    void Clear() noexcept {
        buffer_.clear();
    }

  private:
    std::vector<uint8_t> buffer_;
};

/**
 * @brief Zero-copy binary buffer reader for safe network packet deserialization.
 */
class ByteReader {
  public:
    ByteReader(const uint8_t* data, size_t size) noexcept : data_(data), size_(size), offset_(0) {}

    template <typename T> bool Read(T& out) noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        if (offset_ + sizeof(T) > size_) {
            return false;
        }
        std::memcpy(&out, data_ + offset_, sizeof(T));
        offset_ += sizeof(T);
        return true;
    }

    bool ReadString(std::string& out) {
        uint16_t len = 0;
        if (!Read(len)) {
            return false;
        }
        if (offset_ + len > size_) {
            return false;
        }
        out.assign(reinterpret_cast<const char*>(data_ + offset_), len);
        offset_ += len;
        return true;
    }

    bool ReadBytes(uint8_t* out, size_t count) noexcept {
        if (!out || offset_ + count > size_) {
            return false;
        }
        std::memcpy(out, data_ + offset_, count);
        offset_ += count;
        return true;
    }

    [[nodiscard]] size_t Remaining() const noexcept {
        return size_ > offset_ ? size_ - offset_ : 0;
    }
    [[nodiscard]] bool HasMore() const noexcept {
        return offset_ < size_;
    }
    [[nodiscard]] size_t Offset() const noexcept {
        return offset_;
    }
    void Reset() noexcept {
        offset_ = 0;
    }

  private:
    const uint8_t* data_{nullptr};
    size_t size_{0};
    size_t offset_{0};
};

} // namespace Engine::Network

#endif // ENGINE_NETWORK_BYTE_STREAM_HPP_

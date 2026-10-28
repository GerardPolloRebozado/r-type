#include "Logging/LogBuffer.hpp"

namespace Engine {

LogBuffer::LogBuffer() {
    m_Entries.resize(Capacity);
}

void LogBuffer::Push(LogLevel level, const std::string& message) {
    // Mutex: one thread at a time. lock_guard locks it and unlocks when this function returns.
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_Entries[m_WriteIndex].level = level;
    m_Entries[m_WriteIndex].message = message;
    m_WriteIndex = (m_WriteIndex + 1) % Capacity;
    if (m_Count < Capacity) {
        m_Count++;
    }
}

void LogBuffer::CopyEntriesInOrder(std::vector<LogEntry>& out) const {
    // Same mutex. The guard unlocks on return, before the caller draws.
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::size_t start = 0;

    out.clear();
    if (out.capacity() < m_Count) {
        out.reserve(m_Count);
    }

    // Once the ring is full, the next write slot is the oldest surviving line.
    if (m_Count == Capacity) {
        start = m_WriteIndex;
    }

    for (std::size_t i = 0; i < m_Count; ++i) {
        std::size_t const index = (start + i) % Capacity;
        out.push_back(m_Entries[index]);
    }
}

LogBuffer& GetLogBuffer() {
    static LogBuffer buffer;
    return buffer;
}

} // namespace Engine

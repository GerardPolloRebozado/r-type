/**
 * @file LogBuffer.hpp
 * @brief Fixed-size ring of user-facing log lines for the Editor console.
 *
 * @ref emit pushes a line here only for @ref LogAudience::User. Engine diagnostics
 * stay on the terminal. When the buffer is full, the oldest line is overwritten.
 * The Editor ConsolePanel copies entries out each frame. The copy lock is released
 * before the caller draws.
 */

#ifndef ENGINE_LOGGING_LOGBUFFER_HPP_
#define ENGINE_LOGGING_LOGBUFFER_HPP_

#include "Logging/LogConfig.hpp"
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

namespace Engine {

/**
 * @struct LogEntry
 * @brief One stored log line. The display tag is added by the reader.
 */
struct LogEntry {
    LogLevel level;
    std::string message;
};

/**
 * @class LogBuffer
 * @brief Process-wide ring of the most recent user error messages.
 *
 * @ref Push and @ref CopyEntriesInOrder share one mutex. Capacity is fixed;
 * @ref Push drops the oldest line once full. Engine lines are not stored here.
 */
class LogBuffer {
  public:
    /** @brief How many lines are kept. Older lines are overwritten. */
    static constexpr std::size_t Capacity = 512;

    LogBuffer();

    /**
     * @brief Appends one line, overwriting the oldest when the ring is full.
     * @param level Severity stored with the line.
     * @param message Raw message, without the terminal tag.
     */
    void Push(LogLevel level, const std::string& message);

    /**
     * @brief Copies stored lines from oldest to newest.
     * @param out Cleared, then filled. Capacity is reused when it is already large enough.
     */
    void CopyEntriesInOrder(std::vector<LogEntry>& out) const;

  private:
    std::vector<LogEntry> m_Entries;
    mutable std::mutex m_Mutex;
    std::size_t m_WriteIndex = 0;
    std::size_t m_Count = 0;
};

/**
 * @brief The user-error ring used by @ref emit and the Editor console.
 */
LogBuffer& GetLogBuffer();

} // namespace Engine

#endif // ENGINE_LOGGING_LOGBUFFER_HPP_

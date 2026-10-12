/**
 * @file LogBuffer.hpp
 * @brief Fixed-size ring of recent engine log lines for the Editor console.
 *
 * Written from @ref emit on the main thread. When the buffer is full, the oldest
 * line is overwritten. The Editor ConsolePanel copies entries out each frame.
 */

#ifndef ENGINE_LOGGING_LOGBUFFER_HPP_
#define ENGINE_LOGGING_LOGBUFFER_HPP_

#include "Logging/LogConfig.hpp"
#include <cstddef>
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
 * @brief Process-wide ring of the most recent log messages.
 *
 * Main thread only: the game loop and the Editor UI share one thread, so this
 * type does not lock. Capacity is fixed; @ref Push drops the oldest line once full.
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
    std::size_t m_WriteIndex = 0;
    std::size_t m_Count = 0;
};

/**
 * @brief The single log ring used by @ref emit and the Editor console.
 */
LogBuffer& GetLogBuffer();

} // namespace Engine

#endif // ENGINE_LOGGING_LOGBUFFER_HPP_

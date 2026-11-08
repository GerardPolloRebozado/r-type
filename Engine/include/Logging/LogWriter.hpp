/**
 * @file LogWriter.hpp
 * @brief Background terminal writer for engine log lines.
 *
 * @ref emit moves each line into a bounded queue and returns. One worker thread
 * owns @c std::cout and @c std::cerr. The editor Console does not read this queue.
 */

#ifndef ENGINE_LOGGING_LOGWRITER_HPP_
#define ENGINE_LOGGING_LOGWRITER_HPP_

#include "Logging/LogConfig.hpp"
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <string>
#if !defined(__EMSCRIPTEN__)
#include <thread>
#endif

namespace Engine {

/**
 * @class LogWriter
 * @brief Single consumer of terminal log lines.
 *
 * The caller never blocks on the terminal. When the queue is full, the oldest
 * line is dropped. @ref Shutdown joins the worker once. After that, further
 * lines are written on the caller. The web build does not start this thread.
 */
class LogWriter {
  public:
    /** @brief How many terminal lines are queued. Older lines are dropped. */
    static constexpr std::size_t Capacity = 1024;

    LogWriter();
    ~LogWriter();

    LogWriter(const LogWriter&) = delete;
    LogWriter& operator=(const LogWriter&) = delete;

    /**
     * @brief Queues one line for the worker.
     * @param level Stream and tag chosen by the worker.
     * @param message Raw message, without the terminal tag. Moved from.
     */
    void Enqueue(LogLevel level, std::string message);

    /**
     * @brief Drains the queue and joins the worker.
     * A second call does nothing. Safe when the thread was never started.
     */
    void Shutdown();

  private:
    enum class State { Running, Stopped };

    struct Record {
        LogLevel level;
        std::string message;
    };

    void Run();

    std::mutex m_Mutex;
    std::condition_variable m_Cv;
    std::deque<Record> m_Queue;
#if !defined(__EMSCRIPTEN__)
    std::thread m_Thread;
#endif
    std::size_t m_Dropped = 0;
    bool m_Stopping = false;
    bool m_Started = false;
    bool m_ShutdownCalled = false;
    State m_State = State::Running;
};

/**
 * @brief Sends every terminal log line to stderr.
 *
 * MCP mode keeps stdout for JSON-RPC. Call this before the first log.
 * INFO and DEBUG stay tagged; only the stream changes.
 */
void SetTerminalLogToStderr();

/**
 * @brief Writes one tagged line and flushes that stream.
 * @param level Selects @c std::cerr for errors and @c std::cout otherwise,
 *        unless @ref SetTerminalLogToStderr was called.
 * @param message Raw message, without the terminal tag.
 *
 * Used by the worker's direct fallback and by the web build. Not a call-site API.
 */
void WriteLogLine(LogLevel level, const std::string& message);

/**
 * @brief Queues @p message, or writes it directly when the worker is gone.
 * @param level Severity stored with the line.
 * @param message Raw message. Moved from.
 */
void EnqueueLog(LogLevel level, std::string message);

/**
 * @brief Joins the terminal worker.
 * Called at the end of @c Application shutdown. Later calls are a no-op.
 */
void ShutdownLogging();

} // namespace Engine

#endif // ENGINE_LOGGING_LOGWRITER_HPP_

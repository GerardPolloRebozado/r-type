#include "Logging/LogWriter.hpp"
#include <atomic>
#include <iostream>
#include <utility>

namespace Engine {

namespace {

std::atomic<bool> g_WriterStopped{false};

// One writer for the process, built on the first log. ~LogWriter calls Shutdown.
LogWriter& GetLogWriter() {
    static LogWriter writer;
    return writer;
}

} // namespace

// Synchronous write. Used when there is no worker: web build, thread start
// failed, or the worker has already exited.
void WriteLogLine(LogLevel level, const std::string& message) {
    if (level == LogLevel::Error) {
        std::cerr << "[ERROR] " << message << '\n';
        std::cerr.flush();
        return;
    }
    if (level == LogLevel::Debug) {
        std::cout << "[DEBUG] " << message << '\n';
    } else {
        std::cout << "[INFO] " << message << '\n';
    }
    std::cout.flush();
}

LogWriter::LogWriter() = default;

LogWriter::~LogWriter() {
    Shutdown();
}

void LogWriter::Enqueue(LogLevel level, std::string message) {
#if defined(__EMSCRIPTEN__)
    WriteLogLine(level, message);
#else
    // Mutex guards the queue. unique_lock can unlock before this function returns.
    std::unique_lock<std::mutex> lock(m_Mutex);

    if (m_State == State::Stopped) {
        lock.unlock();
        WriteLogLine(level, message);
        return;
    }
    if (!m_Started) {
        try {
            m_Thread = std::thread([this] { Run(); });
            m_Started = true;
        } catch (...) {
            m_State = State::Stopped;
            g_WriterStopped.store(true, std::memory_order_release);
            lock.unlock();
            WriteLogLine(level, message);
            return;
        }
    }
    if (m_Queue.size() >= Capacity) {
        m_Queue.pop_front();
        m_Dropped++;
    }
    m_Queue.push_back(Record{level, std::move(message)});
    lock.unlock();
    m_Cv.notify_one();
#endif
}

void LogWriter::Shutdown() {
#if defined(__EMSCRIPTEN__)
    g_WriterStopped.store(true, std::memory_order_release);
#else
    std::unique_lock<std::mutex> lock(m_Mutex);

    if (m_ShutdownCalled) {
        return;
    }
    m_ShutdownCalled = true;
    if (!m_Started) {
        m_State = State::Stopped;
        lock.unlock();
        g_WriterStopped.store(true, std::memory_order_release);
        return;
    }
    m_Stopping = true;
    // Unlock before join so the worker can take the mutex and exit.
    lock.unlock();
    m_Cv.notify_one();
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
    g_WriterStopped.store(true, std::memory_order_release);
#endif
}

void LogWriter::Run() {
#if !defined(__EMSCRIPTEN__)
    std::deque<Record> batch;
    std::size_t dropped = 0;
    bool wroteOut = false;
    bool wroteErr = false;

    for (;;) {
        batch.clear();
        dropped = 0;
        {
            std::unique_lock<std::mutex> lock(m_Mutex);

            // condition_variable: sleep until a line arrives. wait unlocks the mutex.
            m_Cv.wait(lock, [this] { return m_Stopping || !m_Queue.empty(); });
            if (m_Stopping && m_Queue.empty()) {
                m_State = State::Stopped;
                break;
            }
            // Hand the queue off so producers can push while we write.
            batch.swap(m_Queue);
            dropped = m_Dropped;
            m_Dropped = 0;
        }

        wroteOut = false;
        wroteErr = false;
        for (const Record& record : batch) {
            if (record.level == LogLevel::Error) {
                std::cerr << "[ERROR] " << record.message << '\n';
                wroteErr = true;
            } else if (record.level == LogLevel::Debug) {
                std::cout << "[DEBUG] " << record.message << '\n';
                wroteOut = true;
            } else {
                std::cout << "[INFO] " << record.message << '\n';
                wroteOut = true;
            }
        }
        if (wroteOut) {
            std::cout.flush();
        }
        if (wroteErr) {
            std::cerr.flush();
        }
        if (dropped > 0) {
            std::cerr << "[ERROR] Logger dropped " << dropped << " lines\n";
            std::cerr.flush();
        }
    }
#endif
}

void EnqueueLog(LogLevel level, std::string message) {
#if defined(__EMSCRIPTEN__)
    WriteLogLine(level, message);
#else
    // Worker is gone. Skip the singleton and write on this thread.
    if (g_WriterStopped.load(std::memory_order_acquire)) {
        WriteLogLine(level, message);
        return;
    }
    GetLogWriter().Enqueue(level, std::move(message));
#endif
}

void ShutdownLogging() {
#if !defined(__EMSCRIPTEN__)
    // Already joined, or the thread failed to start. Do not enter Shutdown again.
    if (g_WriterStopped.load(std::memory_order_acquire)) {
        return;
    }
    GetLogWriter().Shutdown();
#endif
}

} // namespace Engine

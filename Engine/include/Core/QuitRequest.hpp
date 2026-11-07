#ifndef ENGINE_CORE_QUITREQUEST_HPP_
#define ENGINE_CORE_QUITREQUEST_HPP_

namespace Engine {

/**
 * @struct QuitRequest
 * @brief Registry resource asking @ref Application to stop its loop.
 *
 * Application::Run exits after the current frame once @ref requested is true.
 */
struct QuitRequest {
    bool requested = false;
};

} // namespace Engine

#endif // ENGINE_CORE_QUITREQUEST_HPP_

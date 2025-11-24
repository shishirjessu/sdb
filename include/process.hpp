#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>

namespace sdb {

    enum struct Origin { LAUNCHED, LAUNCHED_AND_ATTACHED, ATTACHED };

    enum struct ProcessState { Running, Exited, Stopped, Terminated };

    struct StopReason {
        StopReason(int aStatus) {
            if (WIFEXITED(aStatus)) {
                theStopState = ProcessState::Exited;
                theStatus = WEXITSTATUS(aStatus);
            } else if (WIFSIGNALED(aStatus)) {
                theStopState = ProcessState::Terminated;
                theStatus = WTERMSIG(aStatus);
            } else if (WIFSTOPPED(aStatus)) {
                theStopState = ProcessState::Stopped;
                theStatus = WSTOPSIG(aStatus);
            }
        }

        ProcessState theStopState{};
        std::uint8_t theStatus{};
    };

    class Process {

      public:
        static std::unique_ptr<Process> attach(pid_t aPid);
        static std::unique_ptr<Process>
        launch(const std::filesystem::path& aPath, bool aDebug = true);

        Process() = delete;
        Process(const Process& other) = delete;
        Process& operator=(const Process& other) = delete;

        Process(Process&& other) = delete;
        Process& operator=(Process&& other) = delete;

        pid_t getPid() const;

        StopReason waitOnSignal();
        void resume();

        ~Process();

      private:
        Process(pid_t aPid, Origin origin, bool anIsAttached)
            : thePid{aPid}, theOrigin{origin}, theIsAttached{anIsAttached} {
        }

        pid_t thePid{};
        Origin theOrigin{};
        ProcessState theProcessState{ProcessState::Stopped};
        bool theIsAttached{false};
    };
} // namespace sdb

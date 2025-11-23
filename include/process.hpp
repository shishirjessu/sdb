#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>

namespace sdb {
    enum struct Origin { LAUNCHED, ATTACHED };

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
        launch(const std::filesystem::path& aPath);

        Process() = delete;
        Process(const Process& other) = delete;
        Process& operator=(const Process& other) = delete;

        Process(Process&& other) = delete;
        Process& operator=(Process&& other) = delete;

        pid_t get_pid() const;

        StopReason wait_on_signal();
        void resume();

        ~Process();

      private:
        Process(pid_t aPid, Origin origin) : thePid{aPid}, theOrigin{origin} {
        }

        pid_t thePid{};
        Origin theOrigin{};
        ProcessState theProcessState{ProcessState::Stopped};
    };
} // namespace sdb

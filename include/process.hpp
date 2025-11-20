#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>

namespace sdb {
    enum struct Origin { LAUNCHED, ATTACHED };

    class Process {

      public:
        static std::unique_ptr<Process> attach(pid_t pid);
        static std::unique_ptr<Process>
        launch(const std::filesystem::path& path);

        Process() = delete;
        Process(const Process& other) = delete;
        Process& operator=(const Process& other) = delete;

        Process(Process&& other) = delete;
        Process& operator=(Process&& other) = delete;

        pid_t get_pid() const;

        void wait_on_signal();
        void resume();

        ~Process();

      private:
        Process(pid_t pid, Origin origin) : thePid{pid}, theOrigin{origin} {
        }

        pid_t thePid{};
        Origin theOrigin{};
    };
} // namespace sdb

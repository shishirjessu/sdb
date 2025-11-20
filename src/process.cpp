#include <process.hpp>

#include <cstdio>
#include <stdexcept>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace sdb {
    std::unique_ptr<Process> Process::attach(pid_t pid) {
        if (pid <= 0) {
            throw std::runtime_error("invalid pid");
        }
        if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
            std::perror("attach failed");
        }

        auto myProcess =
            std::unique_ptr<Process>(new Process(pid, Origin::ATTACHED));
        myProcess->wait_on_signal();

        return myProcess;
    }

    std::unique_ptr<Process>
    Process::launch(const std::filesystem::path& path) {
        pid_t pid = fork();
        if (pid < 0) {
            std::perror("fork failed");

        } else if (pid == 0) {
            if (ptrace(PTRACE_TRACEME, pid, nullptr, nullptr) < 0) {
                std::perror("trace failed");
                return {};
            }
            const char* myPathStr = path.c_str();

            if (execlp(myPathStr, myPathStr) < 0) {
                std::perror("exec failed");
                return {};
            }
        }

        auto myProcess =
            std::unique_ptr<Process>(new Process(pid, Origin::LAUNCHED));
        myProcess->wait_on_signal();

        return myProcess;
    }

    void Process::wait_on_signal() {
        int status = 0;
        if ((waitpid(thePid, std::addressof(status), 0)) < 0) {
            std::perror("waitpid failed");
            std::terminate();
        }
    }

    void Process::resume() {
        if (ptrace(PTRACE_CONT, thePid, nullptr, nullptr) < 0) {
            std::perror("continue failed");
            std::terminate();
        }
    }

    pid_t Process::get_pid() const {
        return thePid;
    }

    Process::~Process() {
        if (theOrigin == Origin::LAUNCHED) {
        }
    }

} // namespace sdb
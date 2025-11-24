#include <process.hpp>

#include <cstdio>
#include <error.hpp>
#include <iostream>
#include <pipe.hpp>

#include <stdexcept>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace sdb {

    namespace {
        void exitWithPerror(Pipe& aPipe, const std::string& prefix) {
            auto message = prefix + ": " + std::strerror(errno);
            aPipe.write(reinterpret_cast<std::byte*>(message.data()),
                        message.size());
            exit(-1);
        }

        void handleErrorFromChild(Pipe& aPipe, pid_t aPid) {
            auto data = aPipe.read();
            aPipe.closeRead();

            if (data.size() > 0) {
                waitpid(aPid, nullptr, 0);
                char* myErrorMessage = reinterpret_cast<char*>(data.data());
                std::cout << myErrorMessage << '\n';
                Error::send(
                    std::string(myErrorMessage, myErrorMessage + data.size()));
            }
        }

        bool isLaunched(Origin anOrigin) {
            return anOrigin == Origin::LAUNCHED ||
                   anOrigin == Origin::LAUNCHED_AND_ATTACHED;
        }
    } // namespace

    std::unique_ptr<Process> Process::attach(pid_t aPid) {
        if (aPid <= 0) {
            Error::sendErrno("invalid pid");
        }
        if (ptrace(PTRACE_ATTACH, aPid, nullptr, nullptr) < 0) {
            Error::sendErrno("attach failed\n");
        }

        auto myProcess =
            std::unique_ptr<Process>(new Process(aPid, Origin::ATTACHED, true));
        myProcess->waitOnSignal();

        return myProcess;
    }

    std::unique_ptr<Process> Process::launch(const std::filesystem::path& aPath,
                                             bool aDebug) {
        Pipe myChildToParentPipe{};

        pid_t myPid = fork();
        if (myPid < 0) {
            Error::sendErrno("fork failed\n");

        } else if (myPid == 0) {
            myChildToParentPipe.closeRead();

            if (aDebug and
                ptrace(PTRACE_TRACEME, myPid, nullptr, nullptr) < 0) {
                exitWithPerror(myChildToParentPipe, "trace failed\n");
                return {};
            }
            const char* myPathStr = aPath.c_str();

            if (execlp(myPathStr, myPathStr, nullptr) < 0) {
                exitWithPerror(myChildToParentPipe, "exec failed\n");
                return {};
            }
        }

        myChildToParentPipe.closeWrite();
        handleErrorFromChild(myChildToParentPipe, myPid);

        auto myProcess = std::unique_ptr<Process>(new Process(
            myPid, aDebug ? Origin::LAUNCHED_AND_ATTACHED : Origin::LAUNCHED,
            aDebug));

        if (aDebug) {
            myProcess->waitOnSignal();
        }

        return myProcess;
    }

    StopReason Process::waitOnSignal() {
        int myStatus = 0;
        if ((waitpid(thePid, std::addressof(myStatus), 0)) < 0) {
            Error::sendErrno("waitpid failed\n");
            std::terminate();
        }

        StopReason myStopReason(myStatus);
        theProcessState = myStopReason.theStopState;
        return myStopReason;
    }

    void Process::resume() {
        if (ptrace(PTRACE_CONT, thePid, nullptr, nullptr) < 0) {
            Error::sendErrno("resume failed\n");
            std::terminate();
        }

        theProcessState = ProcessState::Running;
    }

    pid_t Process::getPid() const {
        return thePid;
    }

    Process::~Process() {
        if (thePid == 0) {
            return;
        }

        if (theProcessState == ProcessState::Running) {
            kill(thePid, SIGSTOP);
        }

        if (theIsAttached) {
            ptrace(PTRACE_DETACH, thePid, nullptr, nullptr);
        }

        kill(thePid, isLaunched(theOrigin) ? SIGKILL : SIGCONT);
    }

} // namespace sdb
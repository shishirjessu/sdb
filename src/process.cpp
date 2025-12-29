#include <process.hpp>

#include <cstdio>
#include <error.hpp>
#include <fmt/format.h>
#include <iostream>
#include <pipe.hpp>
#include <register_info.hpp>
#include <types.hpp>

#include <stdexcept>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utility>

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

    std::unique_ptr<Process>
    Process::launch(const std::filesystem::path& aPath, bool aDebug,
                    std::optional<int> aStdoutReplacement) {
        Pipe myChildToParentPipe{};

        pid_t myPid = fork();
        if (myPid < 0) {
            Error::sendErrno("fork failed\n");

        } else if (myPid == 0) {
            myChildToParentPipe.closeRead();

            if (aStdoutReplacement.has_value()) {
                if (dup2(aStdoutReplacement.value(), STDOUT_FILENO) < 0) {
                    exitWithPerror(myChildToParentPipe,
                                   "stdout replacement failed!");
                }
            }

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

        if (theProcessState == ProcessState::Stopped and theIsAttached) {
            readAllRegisters();
        }

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

    void Process::writeFloatingPointRegisters(const user_fpregs_struct& fprs) {
        if (ptrace(PTRACE_SETFPREGS, thePid, nullptr, std::addressof(fprs)) <
            0) {
            Error::send("Could not write floating point registers");
        }
    }

    void Process::writeGeneralPurposeRegisters(const user_regs_struct& gprs) {
        if (ptrace(PTRACE_SETREGS, thePid, nullptr, std::addressof(gprs)) < 0) {
            Error::send("Could not write general purpose registers");
        }
    }

    void Process::writeUserArea(std::size_t anOffset, std::uint64_t aData) {
        if (ptrace(PTRACE_POKEUSER, thePid, anOffset, aData) < 0) {
            int err = errno;
            Error::send(std::string("Could not write register: ") +
                        std::strerror(err));
        }
    }

    void Process::readAllRegisters() {
        auto& myRegisterData = theRegisters.getRegisterData();
        if (ptrace(PTRACE_GETREGS, thePid, nullptr,
                   std::addressof(myRegisterData.regs)) < 0) {
            Error::sendErrno("Could not read general-purpose registers");
        }

        if (ptrace(PTRACE_GETFPREGS, thePid, nullptr,
                   std::addressof(myRegisterData.i387)) < 0) {
            Error::sendErrno("Could not read floating-point registers");
        }

        for (int i = 0; i < 8; ++i) {
            RegisterId myRegisterId =
                RegisterId{toUnderlying(RegisterId::dr0) + i};
            auto& myRegisterInfo = findRegisterById(myRegisterId);

            errno = 0;
            std::int64_t myRegisterValue = ptrace(
                PTRACE_PEEKUSER, thePid, myRegisterInfo.theOffset, nullptr);

            if (errno != 0) {
                Error::sendErrno("Could not read debug register " +
                                 std::to_string(i));
            }

            myRegisterData.u_debugreg[i] = myRegisterValue;
        }
    }

    VirtualAddress Process::getPc() const {
        return VirtualAddress{std::get<uint64_t>(
            theRegisters.read(findRegisterById(RegisterId::rip)))};
    }

    BreakpointSite& Process::createBreakpointSite(VirtualAddress anAddress) {
        if (theStoppoints.contains_address(anAddress)) [[unlikely]] {
            Error::send(fmt::format("Trying to create breakpoint at address {}",
                                    std::to_underlying(anAddress)));
        }

        return theStoppoints.push(
            std::make_unique<BreakpointSite>(*this, anAddress));
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
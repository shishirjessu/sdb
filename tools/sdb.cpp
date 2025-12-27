#include <CLI/CLI.hpp>
#include <editline/readline.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iostream>
#include <process.hpp>
#include <ranges>
#include <register_write.hpp>
#include <string>
#include <unistd.h>
#include <utils.hpp>
#include <vector>

void add_reg_reading(CLI::App& theRepl, sdb::Process& aProcess) {
    using namespace sdb;

    auto reg = theRepl.get_subcommand("reg");

    std::string myRegisterName{};
    auto reg_read = reg->add_subcommand("read", "Read from register");
    reg_read->add_option("register", myRegisterName)->required();

    reg_read->callback([&]() {
        // TODO: implement support for reading all registers
        auto& myRegisters = aProcess.getRegisters();
        try {
            const RegisterInfo& myRegister = findRegisterByName(myRegisterName);
            auto myRegisterValue = myRegisters.read(myRegister);

            auto myFormatFn = [&](auto t) {
                if constexpr (std::is_floating_point_v<decltype(t)>) {
                    return fmt::format("{}", t);
                } else if constexpr (std::is_integral_v<decltype(t)>) {
                    return fmt::format("{:#0{}x}", t, sizeof(t) * 2 + 2);
                } else {
                    return fmt::format("[{:#04x}]", fmt::join(t, ","));
                }
            };

            fmt::print("{}:\t{}\n", myRegisterName,
                       std::visit(myFormatFn, myRegisterValue));
        } catch (const sdb::Error& anError) {
            std::cerr << "Invalid register name\n";
        }
    });
}

void add_reg_writing(CLI::App& aRepl, sdb::Process& aProcess) {
    auto reg = aRepl.get_subcommand("reg");

    std::string myRegister{};
    std::string myValue{};

    auto reg_write = reg->add_subcommand("write", "Write to register");
    reg_write->add_option("register", myRegister)->required();
    reg_write->add_option("value", myValue)->required();

    reg_write->callback(
        [&]() { handleRegisterWrite(aProcess, myRegister, myValue); });
}

void add_continue(CLI::App& aRepl, sdb::Process& aProcess) {
    auto continue_cmd = aRepl.add_subcommand("c");

    continue_cmd->callback([&]() {
        aProcess.resume();
        aProcess.waitOnSignal();
    });
}

void readInput(std::unique_ptr<sdb::Process>& aProcess) {
    CLI::App myRepl;

    add_continue(myRepl, *aProcess);

    myRepl.add_subcommand("reg", "Register operations");
    add_reg_reading(myRepl, *aProcess);
    add_reg_writing(myRepl, *aProcess);

    char* myLine = nullptr;
    while ((myLine = readline("sdb> ")) != nullptr) {
        std::string myLineStr{};

        if (myLine == std::string_view("")) {
            if (history_length > 0) {
                myLineStr = history_list()[history_length - 1]->line;
            }
        } else {
            myLineStr = myLine;
            add_history(myLine);
        }

        try {
            myRepl.parse(myLineStr);
        } catch (const CLI::ParseError& e) {
            std::cerr << e.what() << '\n';
        }

        myRepl.clear();
        free(myLine);
    }
}

int main(int argc, char** argv) {
    CLI::App mySdb{"Debugger!"};

    pid_t myPid{};
    std::string myFilename{};

    auto myOptionGroup = mySdb.add_option_group(
        "Filename or PID", "Choose either filename or pid to debug.");

    auto myPidOpt =
        myOptionGroup->add_option("-p,--pid", myPid, "A pid to attach to");
    auto myFileOpt = myOptionGroup->add_option("file", myFilename,
                                               "An executable to launch");

    try {
        mySdb.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return mySdb.exit(e);
    }

    std::unique_ptr<sdb::Process> myProcess;

    if (myPidOpt->count() > 0) {
        myProcess = sdb::Process::attach(myPid);
        readInput(myProcess);
    } else if (myFileOpt->count() > 0) {
        myProcess = sdb::Process::launch(myFilename);
        readInput(myProcess);
    }
}

#include <CLI/CLI.hpp>
#include <editline/readline.h> // remove the copts change
#include <iostream>
#include <process.hpp>
#include <string>
#include <unistd.h> // for pid_t
#include <utils.hpp>

#include <iostream>
#include <ranges>
#include <string>
#include <vector>

void readInput(std::unique_ptr<sdb::Process>& myProcess) {
    char* line = nullptr;
    while ((line = readline("sdb> ")) != nullptr) {
        std::string line_str{};

        if (line == std::string_view("")) {
            if (history_length > 0) {
                line_str = history_list()[history_length - 1]->line;
            }
        } else {
            line_str = line;
            add_history(line);
        }

        auto args = sdb::split(line_str, ' ');
        std::string command = args[0];

        if (sdb::isPrefix("continue", command)) {
            myProcess->resume();
            myProcess->waitOnSignal();
        } else {
            std::cerr << "unknown command\n";
        }

        free(line);
    }
}

int main(int argc, char** argv) {
    CLI::App sdb{"Debugger!"};

    pid_t pid{};
    std::string filename{};

    auto option_group = sdb.add_option_group(
        "Filename or PID", "Choose either filename or pid to debug.");

    auto pid_opt =
        option_group->add_option("-p,--pid", pid, "A pid to attach to");
    auto file_opt =
        option_group->add_option("file", filename, "An executable to launch");

    option_group->require_option(1);

    try {
        sdb.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return sdb.exit(e);
    }

    std::unique_ptr<sdb::Process> myProcess;

    if (pid_opt->count() > 0) {
        myProcess = sdb::Process::attach(pid);
    } else if (file_opt->count() > 0) {
        myProcess = sdb::Process::launch(filename);
    }

    readInput(myProcess);
}

#include <CLI/CLI.hpp>
#include <editline/readline.h> // remove the copts change
#include <iostream>
#include <process.hpp>
#include <string>
#include <unistd.h> // for pid_t

#include <iostream>
#include <ranges>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;

    for (auto&& part : str | std::views::split(delimiter)) {
        // Convert subrange of chars to string
        result.emplace_back(part.begin(), part.end());
    }

    return result;
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

        auto args = split(line_str, ' ');
        std::string command = args[0];

        if (command.starts_with("continue")) {
            myProcess->resume();
            myProcess->wait_on_signal();
        } else {
            std::cerr << "unknown command";
        }

        free(line);
    }

    /*
    while (user input exists)
        if line is empty
            use last line if exists

        handle command
        add history
        free line
    */
}

#pragma once

#include <CLI/CLI.hpp>
#include <process.hpp>

namespace sdb {
    void add_memory_commands(CLI::App& aRepl, sdb::Process& aProcess);
} // namespace sdb

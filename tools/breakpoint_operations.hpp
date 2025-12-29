#pragma once

#include <CLI/CLI.hpp>
#include <process.hpp>

namespace sdb {
    void add_breakpoint_operations(CLI::App& aRepl, sdb::Process& aProcess);
} // namespace sdb

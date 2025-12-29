#pragma once

#include <cstdint>
#include <elf.h>
#include <filesystem>

#include <sys/types.h>

#include <process.hpp>

namespace sdb::test {

    std::int64_t get_section_load_bias(const std::string& aFile,
                                       Elf64_Addr aFileAddress);

    std::int64_t get_entry_point_offset(std::filesystem::path aPath);

    VirtualAddress get_load_address(pid_t aPid, std::int64_t anOffset);

} // namespace sdb::test

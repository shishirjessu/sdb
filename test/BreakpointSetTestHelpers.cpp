#include "BreakpointSetTestHelpers.hpp"

#include <elf.h>
#include <error.hpp>

#include <fstream>
#include <regex>
#include <signal.h>
#include <stdio.h>
#include <string>

namespace sdb::test {

    std::int64_t get_section_load_bias(const std::string& aFile,
                                       Elf64_Addr aFileAddress) {
        auto command = std::string("readelf -WS ") + aFile;
        auto pipe = popen(command.c_str(), "r");

        std::regex text_regex(R"(PROGBITS\s+(\w+)\s+(\w+)\s+(\w+))");
        char* line = nullptr;
        std::size_t len = 0;

        while (getline(&line, &len, pipe) != -1) {
            std::cmatch groups;
            if (std::regex_search(line, groups, text_regex)) {
                auto address = std::stol(groups[1], nullptr, 16);
                auto offset = std::stol(groups[2], nullptr, 16);
                auto size = std::stol(groups[3], nullptr, 16);

                if (address <= aFileAddress &&
                    aFileAddress < (address + size)) {
                    free(line);
                    pclose(pipe);
                    return address - offset;
                }
            }
            free(line);
            line = nullptr;
        }

        pclose(pipe);
        Error::send("Could not find section load bias");
    }

    std::int64_t get_entry_point_offset(std::filesystem::path aPath) {
        std::ifstream elf_file(aPath);
        Elf64_Ehdr header;

        elf_file.read(reinterpret_cast<char*>(&header), sizeof(header));

        auto entry_file_address = header.e_entry;
        auto load_bias =
            get_section_load_bias(aPath.string(), entry_file_address);

        return entry_file_address - load_bias;
    }

    VirtualAddress get_load_address(pid_t aPid, std::int64_t anOffset) {
        std::ifstream maps("/proc/" + std::to_string(aPid) + "/maps");
        std::regex map_regex(R"((\w+)-\w+ ..(.). (\w+))");
        std::string data;

        while (std::getline(maps, data)) {
            std::smatch groups;
            std::regex_search(data, groups, map_regex);

            if (groups[2] == 'x') {
                auto low_range = std::stol(groups[1], nullptr, 16);
                auto file_offset = std::stol(groups[3], nullptr, 16);

                return VirtualAddress(anOffset - file_offset + low_range);
            }
        }

        Error::send("Could not find load address");
    }

} // namespace sdb::test

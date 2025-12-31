#include <CLI/CLI.hpp>
#include <process.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <memory_operations.hpp>
#include <register_write.hpp>

namespace sdb {
    namespace {
        void add_memory_read(CLI::App& aRepl, sdb::Process& aProcess) {
            auto mem = aRepl.get_subcommand("memory");
            auto mem_read =
                mem->add_subcommand("read", "Read memory at the given address");

            CLI::Option* myAddressOpt = mem_read->add_option("address")
                                            ->required()
                                            ->capture_default_str();

            CLI::Option* myNumBytesOpt = mem_read->add_option("numBytes")
                                             ->default_val("32")
                                             ->capture_default_str();

            mem_read->callback([=, &aProcess]() {
                std::string myAddressStr = myAddressOpt->as<std::string>();
                std::string myNumBytesStr = myNumBytesOpt->as<std::string>();

                auto myOptionalAddr =
                    sdb::toIntegral<std::uint64_t>(myAddressStr);
                if (!myOptionalAddr) {
                    fmt::print(
                        stderr,
                        "Breakpoint command expects address in hexadecimal, "
                        "prefixed with '0x'\n");
                    return;
                }

                std::size_t myNumBytes =
                    sdb::toIntegral<std::size_t>(myNumBytesStr).value();

                sdb::VirtualAddress myAddr{*myOptionalAddr};
                auto data = readMemory(aProcess.getPid(), myAddr, myNumBytes);

                for (std::size_t i = 0; i < data.size(); i += 16) {
                    auto start = data.begin() + i;
                    auto end = data.begin() + std::min(i + 16, data.size());
                    fmt::print("{:#016x}: {:02x}\n",
                               std::to_underlying(myAddr) + i,
                               fmt::join(start, end, " "));
                }
            });
        }

        void add_memory_write(CLI::App& aRepl, sdb::Process& aProcess) {
            auto mem = aRepl.get_subcommand("memory");
            auto mem_write = mem->add_subcommand(
                "write", "Write memory to the given address");

            CLI::Option* myAddressOpt = mem_write->add_option("address")
                                            ->required()
                                            ->capture_default_str();

            CLI::Option* myMemoryVecOpt = mem_write->add_option("memoryToWrite")
                                              ->required()
                                              ->capture_default_str();

            mem_write->callback([=, &aProcess]() {
                std::string myAddressStr = myAddressOpt->as<std::string>();
                std::string myMemoryVecStr = myMemoryVecOpt->as<std::string>();

                auto myOptionalAddr =
                    sdb::toIntegral<std::uint64_t>(myAddressStr);
                if (!myOptionalAddr) {
                    fmt::print(stderr, "Breakpoint command expects address in "
                                       "hexadecimal, prefixed with '0x'\n");
                    return;
                }

                auto data = toVectorDynamic(myMemoryVecStr);
                writeMemory(aProcess.getPid(), VirtualAddress{*myOptionalAddr},
                            {data->data(), data->size()});
            });
        }

    } // namespace

    void add_memory_commands(CLI::App& aRepl, sdb::Process& aProcess) {
        aRepl.add_subcommand("memory", "Memory operations");

        add_memory_read(aRepl, aProcess);
        add_memory_write(aRepl, aProcess);
    }
} // namespace sdb

#include "breakpoint_operations.hpp"

#include <register_write.hpp>

#include <cstdint>
#include <string>

#include <fmt/core.h>

#include <process.hpp>
#include <types.hpp>

namespace sdb {
    namespace {

        void add_breakpoint_listing(CLI::App& aRepl, sdb::Process& aProcess) {
            auto bp = aRepl.get_subcommand("breakpoint");
            auto bp_list = bp->add_subcommand(
                "list", "List all breakpoints in the current process");

            bp_list->callback([&aProcess]() {
                auto& myBreakpointSites = aProcess.getBreakpointSites();
                if (myBreakpointSites.empty()) {
                    fmt::print("No breakpoints set\n");
                } else {
                    myBreakpointSites.forEach([](auto& aSite) {
                        fmt::print("{}: address = {:#x}, {}\n",
                                   std::to_underlying(aSite.getId()),
                                   std::to_underlying(aSite.getAddress()),
                                   aSite.isEnabled() ? "enabled" : "disabled");
                    });
                }
            });
        }

        void add_breakpoint_setting(CLI::App& aRepl, sdb::Process& aProcess) {
            auto bp = aRepl.get_subcommand("breakpoint");
            auto bp_set = bp->add_subcommand(
                "set", "Set a breakpoint at the given address");

            CLI::Option* myAddressOpt = bp_set->add_option("address")
                                            ->required()
                                            ->capture_default_str();

            bp_set->callback([=, &aProcess]() {
                const std::string& myAddressStr =
                    myAddressOpt->as<std::string>();

                auto myOptionalAddr =
                    sdb::toIntegral<std::uint64_t>(myAddressStr);
                if (!myOptionalAddr) {
                    fmt::print(stderr, "Breakpoint command expects address in "
                                       "hexadecimal, prefixed with '0x'\n");
                    return;
                }

                sdb::VirtualAddress myAddr{*myOptionalAddr};
                aProcess.createBreakpointSite(myAddr).enable();
            });
        }

        void add_breakpoint_enable(CLI::App& aRepl, sdb::Process& aProcess) {
            auto bp = aRepl.get_subcommand("breakpoint");
            auto bp_enable = bp->add_subcommand(
                "enable", "Enable a breakpoint with the given ID");

            CLI::Option* myIdOpt =
                bp_enable->add_option("id")->required()->capture_default_str();

            bp_enable->callback([=, &aProcess]() {
                const std::string& myIdStr = myIdOpt->as<std::string>();

                auto myOptionalId = sdb::toIntegral<std::uint32_t>(myIdStr);
                auto& myBreakpointSites = aProcess.getBreakpointSites();
                myBreakpointSites
                    .getById(sdb::BreakpointSite::IdTypeT{*myOptionalId})
                    .enable();
            });
        }

        void add_breakpoint_disable(CLI::App& aRepl, sdb::Process& aProcess) {
            auto bp = aRepl.get_subcommand("breakpoint");
            auto bp_disable = bp->add_subcommand(
                "disable", "Disable a breakpoint with the given ID");

            CLI::Option* myIdOpt =
                bp_disable->add_option("id")->required()->capture_default_str();

            bp_disable->callback([=, &aProcess]() {
                const std::string& myIdStr = myIdOpt->as<std::string>();

                auto myOptionalId = sdb::toIntegral<std::uint32_t>(myIdStr);
                auto& myBreakpointSites = aProcess.getBreakpointSites();
                myBreakpointSites
                    .getById(sdb::BreakpointSite::IdTypeT{*myOptionalId})
                    .disable();
            });
        }

        void add_breakpoint_delete(CLI::App& aRepl, sdb::Process& aProcess) {
            auto bp = aRepl.get_subcommand("breakpoint");
            auto bp_delete = bp->add_subcommand(
                "delete", "Delete a breakpoint with the given ID");

            CLI::Option* myIdOpt =
                bp_delete->add_option("id")->required()->capture_default_str();

            bp_delete->callback([=, &aProcess]() {
                const std::string& myIdStr = myIdOpt->as<std::string>();

                auto myOptionalId = sdb::toIntegral<std::uint32_t>(myIdStr);
                auto& myBreakpointSites = aProcess.getBreakpointSites();
                myBreakpointSites.removeById(
                    sdb::BreakpointSite::IdTypeT{*myOptionalId});
            });
        }

    } // namespace

    void add_breakpoint_operations(CLI::App& aRepl, sdb::Process& aProcess) {
        aRepl.add_subcommand("breakpoint", "Breakpoint operations");

        add_breakpoint_listing(aRepl, aProcess);
        add_breakpoint_setting(aRepl, aProcess);
        add_breakpoint_enable(aRepl, aProcess);
        add_breakpoint_disable(aRepl, aProcess);
        add_breakpoint_delete(aRepl, aProcess);
    }

} // namespace sdb

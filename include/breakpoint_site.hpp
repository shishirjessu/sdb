#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <types.hpp>

#include <sys/ptrace.h>

namespace sdb {

    enum class BreakpointSiteId : std::uint32_t {};

    inline constexpr BreakpointSiteId operator++(BreakpointSiteId& anId) {
        anId = BreakpointSiteId(toUnderlying(anId) + 1);
        return anId;
    }

    inline constexpr BreakpointSiteId operator+(BreakpointSiteId anId,
                                                std::uint64_t anInc) noexcept {
        return BreakpointSiteId(toUnderlying(anId) + anInc);
    }

    inline constexpr BreakpointSiteId getNextId() {
        static BreakpointSiteId myId{0};
        return ++myId;
    }

    class Process;

    class BreakpointSite {
      public:
        static constexpr std::uint64_t INT3 = 0xcc;

        using IdTypeT = BreakpointSiteId;

        BreakpointSite(Process& aProcess, VirtualAddress anAddress)
            : theProcess{aProcess}, theAddress{anAddress},
              theSavedData{}, theId{getNextId()} {
        }

        BreakpointSite() = delete;

        BreakpointSite(const BreakpointSite& other) = delete;
        BreakpointSite& operator=(const BreakpointSite& other) = delete;

        BreakpointSite(BreakpointSite&& other) = delete;
        BreakpointSite& operator=(BreakpointSite&& other) = delete;

        void enable();
        void disable();
        bool isEnabled() const;

        IdTypeT getId() const;
        VirtualAddress getAddress() const;

      private:
        bool theEnabled{false};

        Process& theProcess;
        VirtualAddress theAddress;
        std::byte theSavedData;
        BreakpointSiteId theId;

        std::uint64_t getDataAtAddress();
        void putDataAtAddress(std::uint64_t myDataToWrite);
    };
} // namespace sdb
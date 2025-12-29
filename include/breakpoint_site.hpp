#pragma once

#include <cstdint>
#include <types.hpp>

namespace sdb {

    namespace {
        enum class BreakpointSiteId : std::uint32_t {};

        BreakpointSiteId operator++(BreakpointSiteId& anId) {
            anId = BreakpointSiteId(toUnderlying(anId) + 1);
            return anId;
        }

        BreakpointSiteId operator+(BreakpointSiteId anId,
                                   std::uint64_t anInc) noexcept {
            return BreakpointSiteId(toUnderlying(anId) + anInc);
        }

        BreakpointSiteId getNextId() {
            static BreakpointSiteId myId{0};
            return ++myId;
        }
    } // namespace

    class Process;

    class BreakpointSite {
      public:
        using IdTypeT = BreakpointSiteId;

        BreakpointSite(Process& aProcess, VirtualAddress anAddress)
            : theProcess{aProcess}, theAddress{anAddress},
              theSavedData{}, theId{getNextId()} {
        }

        BreakpointSite() = delete;

        ~BreakpointSite() {
            if (theEnabled) {
                disable();
            }
        }

        BreakpointSite(const BreakpointSite& other) = delete;
        BreakpointSite& operator=(const BreakpointSite& other) = delete;

        BreakpointSite(BreakpointSite&& other) = delete;
        BreakpointSite& operator=(BreakpointSite&& other) = delete;

        IdTypeT getId() const {
            return theId;
        }

        void enable() {
        }

        void disable() {
        }

        bool isEnabled() const {
            return theEnabled;
        }

        VirtualAddress getAddress() const {
            return theAddress;
        }

      private:
        bool theEnabled{false};

        Process& theProcess;
        VirtualAddress theAddress;
        std::byte theSavedData;
        BreakpointSiteId theId;
    };
} // namespace sdb
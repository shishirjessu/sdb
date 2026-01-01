#include <breakpoint_site.hpp>
#include <process.hpp>

namespace sdb {
    void BreakpointSite::enable() {
        if (theEnabled) {
            return;
        }

        std::uint64_t myData = getDataAtAddress();

        theSavedData = static_cast<std::byte>(myData & 0xff);

        std::uint64_t myDataToWrite = (myData & ~0xff) | INT3;
        putDataAtAddress(myDataToWrite);

        theEnabled = true;
    }

    void BreakpointSite::disable() {
        if (!theEnabled) {
            Error::send(fmt::format(
                "Disabling breakpoint at already disabled address {}",
                std::to_underlying(theAddress)));
        }

        std::uint64_t myData = getDataAtAddress();
        std::uint64_t myDataToWrite =
            (myData & ~0xff) | static_cast<std::uint8_t>(theSavedData);
        theSavedData = std::byte{0};

        putDataAtAddress(myDataToWrite);

        theEnabled = false;
    }

    std::uint64_t BreakpointSite::getDataAtAddress() {
        errno = 0;
        std::uint64_t myData = ptrace(PTRACE_PEEKDATA, theProcess.getPid(),
                                      std::to_underlying(theAddress), nullptr);

        if (errno != 0) {
            Error::sendErrno(
                fmt::format("Retrieving memory at address {:#04x} failed\n",
                            std::to_underlying(theAddress)));
        }

        return myData;
    }

    void BreakpointSite::putDataAtAddress(std::uint64_t myDataToWrite) {
        if (ptrace(PTRACE_POKEDATA, theProcess.getPid(),
                   std::to_underlying(theAddress), myDataToWrite) < 0) {
            Error::sendErrno(
                fmt::format("Modifying memory at address {} failed",
                            std::to_underlying(theAddress)));
        }
    }

    BreakpointSite::IdTypeT BreakpointSite::getId() const {
        return theId;
    }

    bool BreakpointSite::isEnabled() const {
        return theEnabled;
    }

    VirtualAddress BreakpointSite::getAddress() const {
        return theAddress;
    }

    std::byte BreakpointSite::getSavedData() const {
        return theSavedData;
    }

} // namespace sdb
#pragma once

#include <bit.hpp>
#include <types.hpp>
#include <vector>

#include <sys/uio.h>

namespace sdb {
    std::vector<std::byte> readMemory(pid_t aPid, VirtualAddress anAddress,
                                      std::size_t anAmount) {
        std::vector<std::byte> myResult(anAmount);

        iovec myLocalDesc{myResult.data(), myResult.size()};

        std::vector<iovec> myRemoteDescs{};
        while (anAmount > 0) {
            std::size_t myBytesToNextPage =
                0x1000 - (std::to_underlying(anAddress) & 0xfff);
            std::size_t mySizeOfNextChunk =
                std::min(anAmount, myBytesToNextPage);

            myRemoteDescs.emplace_back(
                reinterpret_cast<void*>(std::to_underlying(anAddress)),
                mySizeOfNextChunk);

            anAddress += mySizeOfNextChunk;
            anAmount -= mySizeOfNextChunk;
        }

        if (process_vm_readv(aPid, &myLocalDesc, 1, myRemoteDescs.data(),
                             myRemoteDescs.size(), 0) < 0) {
            Error::sendErrno("Failed to read process memory");
        }

        return myResult;
    }

    void writeMemory(pid_t aPid, VirtualAddress anAddress,
                     std::span<const std::byte> aMemory) {
        std::size_t myNumBytesWritten = 0;
        while (myNumBytesWritten < aMemory.size()) {
            std::size_t myNumBytesRemaining =
                aMemory.size() - myNumBytesWritten;

            std::uint64_t myWord;
            if (myNumBytesRemaining >= 8) {
                myWord = fromBytes<std::uint64_t>(aMemory.data() +
                                                  myNumBytesWritten);
            } else [[unlikely]] {
                // Writing under 8 bytes. In this case, we grab the memory
                // already at the address
                auto myBytes =
                    readMemory(aPid, anAddress + myNumBytesWritten, 8);
                auto myWordData =
                    reinterpret_cast<char*>(std::addressof(myWord));

                // then copy the bytes we want to put into the first part
                // of the word
                std::memcpy(myWordData, aMemory.data() + myNumBytesWritten,
                            myNumBytesRemaining);

                // then copy the remaining bytes back in. We could just
                // overwrite the first part in theory, but this is safer
                // for cross-endian cases which could come up in remote
                // debugging for example
                std::memcpy(myWordData + myNumBytesRemaining,
                            myBytes.data() + myNumBytesRemaining,
                            8 - myNumBytesRemaining);
            }

            if (ptrace(PTRACE_POKEDATA, aPid,
                       std::to_underlying(anAddress) + myNumBytesWritten,
                       myWord) < 0) {
                Error::sendErrno("Failed to write data to memory");
            }

            myNumBytesWritten += 8;
        }
    }

} // namespace sdb
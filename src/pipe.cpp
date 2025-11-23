#include <pipe.hpp>

#include <fcntl.h>
#include <unistd.h>

namespace sdb {

    Pipe::Pipe() {
        if (pipe2(theFDs.data(), O_CLOEXEC) < 0) {
            Error::sendErrno("pipe creation failed");
        }
    }

    std::vector<std::byte> Pipe::read() {
        std::array<std::byte, CAPACITY> aBuffer;

        size_t myNumCharsRead =
            ::read(theFDs[READ_FD_IDX], aBuffer.data(), CAPACITY);

        if (myNumCharsRead < 0) {
            Error::sendErrno("failed to read from pipe");
        }

        return std::vector<std::byte>(aBuffer.begin(),
                                      aBuffer.begin() + myNumCharsRead);
    }

    void Pipe::write(std::byte* aSrc, std::size_t aNumBytes) {
        if (::write(theFDs[WRITE_FD_IDX], aSrc, aNumBytes) < 0) {
            Error::sendErrno("failed to write to pipe");
        }
    }

    void Pipe::closeRead() {
        closeFd(READ_FD_IDX);
    }

    void Pipe::closeWrite() {
        closeFd(WRITE_FD_IDX);
    }

    void Pipe::closeFd(int aFdIdx) {
        if (theFDs.at(aFdIdx) != -1) {
            ::close(theFDs.at(aFdIdx));
            theFDs[aFdIdx] = -1;
        }
    }

    Pipe::~Pipe() {
        closeRead();
        closeWrite();
    }

} // namespace sdb

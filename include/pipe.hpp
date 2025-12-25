#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <error.hpp>
#include <vector>

namespace sdb {

    class Pipe {
      public:
        explicit Pipe(bool aCloseOnExec = true);

        std::vector<std::byte> read();
        void write(std::byte* aSrc, std::size_t aNumBytes);

        void closeRead();
        void closeWrite();

        int getRead() const {
            return theFDs[READ_FD_IDX];
        }

        int getWrite() const {
            return theFDs[WRITE_FD_IDX];
        }

        ~Pipe();

      private:
        bool theCloseOnExec{true};

        static constexpr std::size_t CAPACITY{1024};
        static constexpr std::size_t READ_FD_IDX{0};
        static constexpr std::size_t WRITE_FD_IDX{1};

        std::array<int, 2> theFDs;

        void closeFd(int aFdIdx);
    };

} // namespace sdb

#pragma once

#include <cstring>
#include <stdexcept>

namespace sdb {

    class Error : public std::runtime_error {
      public:
        [[noreturn]] static void send(const std::string& anError) {
            throw Error(anError);
        }

        [[noreturn]] static void sendErrno(const std::string& aPrefix) {
            throw Error(aPrefix + std::strerror(errno));
        }

      private:
        Error(const std::string& anError) : std::runtime_error{anError} {
        }
    };

} // namespace sdb
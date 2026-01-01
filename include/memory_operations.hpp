#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <types.hpp>

namespace sdb {
    std::vector<std::byte> readMemory(pid_t aPid, VirtualAddress anAddress,
                                      std::size_t anAmount);

    void writeMemory(pid_t aPid, VirtualAddress anAddress,
                     std::span<const std::byte> aMemory);

} // namespace sdb
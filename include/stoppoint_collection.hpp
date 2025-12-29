#pragma once

#include <algorithm>
#include <fmt/format.h>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace sdb {
    template <typename StoppointT>
    class StoppointCollection {
      public:
        constexpr StoppointCollection() = default;

        StoppointT& push(std::unique_ptr<StoppointT> aStoppoint) {
            theStoppoints.emplace_back(std::move(aStoppoint));
            return *theStoppoints.back();
        }

        bool contains_id(StoppointT::IdTypeT anId) const {
            return findById(anId) != theStoppoints.end();
        }

        bool contains_address(VirtualAddress anAddress) const {
            return findByAddress(anAddress) != theStoppoints.end();
        }

        bool stoppointEnabledAtAddress(VirtualAddress anAddress) const {
            auto it = findByAddress(anAddress);
            return it != theStoppoints.end() and it->isEnabled();
        }

        template <typename Self>
        decltype(auto) getById(this Self&& self, StoppointT::IdTypeT anId) {
            auto myIt = self.findById(anId);

            if (myIt == self.theStoppoints.end()) {
                Error::send(fmt::format("Invalid breakpoint ID {}",
                                        std::to_underlying(anId)));
            }

            return **myIt;
        }

        template <typename Self>
        decltype(auto) getByAddress(this Self&& self,
                                    VirtualAddress anAddress) {
            auto myIt = self.findByAddress(anAddress);

            if (myIt == self.theStoppoints.end()) {
                Error::send(fmt::format("Invalid stoppoint address {}",
                                        std::to_underlying(anAddress)));
            }

            return **myIt;
        }

        void removeById(StoppointT::IdTypeT anId) {
            auto it = findById(anId);
            if (it == theStoppoints.end()) {
                Error::send(fmt::format(
                    "Trying to delete stoppoint with nonexistent ID {}",
                    std::to_underlying(anId)));
            }

            theStoppoints.erase(it);
        }
        void removeByAddress(VirtualAddress anAddress) {
            auto it = findByAddress(anAddress);
            if (it == theStoppoints.end()) {
                Error::send(fmt::format(
                    "Trying to delete stoppoint with nonexistent address {}",
                    std::to_underlying(anAddress)));
            }

            theStoppoints.erase(it);
        }

        template <typename Self, typename F>
        void forEach(this Self&& self, F aFunction) {
            for (auto&& myStoppoint : self.theStoppoints) {
                aFunction(*myStoppoint);
            }
        }

        std::size_t size() const {
            return theStoppoints.size();
        }
        bool empty() const {
            return theStoppoints.empty();
        }

      private:
        std::vector<std::unique_ptr<StoppointT>> theStoppoints;

        auto findById(StoppointT::IdTypeT anId) const {
            return std::ranges::find_if(
                theStoppoints,
                [anId](const std::unique_ptr<StoppointT>& aStoppoint) {
                    return aStoppoint->getId() == anId;
                });
        }

        auto findByAddress(VirtualAddress anAddress) const {
            return std::ranges::find_if(
                theStoppoints,
                [anAddress](const std::unique_ptr<StoppointT>& aStoppoint) {
                    return aStoppoint->getAddress() == anAddress;
                });
        }
    };
} // namespace sdb
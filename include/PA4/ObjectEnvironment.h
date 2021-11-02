#pragma once
#include <vector>
#include <map>

namespace semant {
    class ObjectEnvironment {
    public:
        void EnterScope() {
            stack_.emplace_back();
        }

        void ExitScope() {
            stack_.pop_back();
        }

        void AddIdWithType(const std::string& id, const std::string& type) {
            stack_.back()[id] = type;
        }

        void PopId(const std::string& id) {
            stack_.back().erase(id);
        }

        bool IsDefined(const std::string& id) {
            for (const auto& scope: stack_) {
                if (scope.find(id) != scope.end()) return true;
            }
            return false;
        }

        bool IsDefinedInLocalScope(const std::string& id) {
            return stack_.back().find(id) != stack_.back().end();
        }

        std::string GetType(const std::string& id) {
            for (std::size_t i = stack_.size(); i > 0; --i) {
                const auto& scope = stack_[i - 1];
                auto type = scope.find(id);
                if (type != scope.end()) return type->second;
            }
            throw std::runtime_error("Undeclared identifier " + id + ".");
        }

    private:
        std::vector<std::map<std::string, std::string>> stack_;
    };
}
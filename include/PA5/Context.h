#pragma once
namespace backend {
    class Context {
    public:
        void SetClassName(const std::string& name) {
            class_name_ = name;
        }

        void EnterScope() {
            local_to_offset.emplace_back();
        }

        void ExitScope() {
            local_to_offset.pop_back();
        }

        void AddAttrs(const std::vector<parser::AttrFeature>& attrs) {
            attr_to_offset.clear();
            for (const auto& attr: attrs) {
                attr_to_offset[attr.id.id] = 12 + 4 * attr_to_offset.size();
            }
        }

        void EnterMethod(const parser::MethodFeature& method) {
            method_name_ = method.id.id;
            arg_to_offset.clear();
            for (const auto& formal: method.params) {
                arg_to_offset[formal.id.id] = 16 + 4 * arg_to_offset.size();
            }
        }

        void AddLocalId(const std::string& id) {
            local_to_offset.back()[id] = local_to_offset.back().size() * 4;
        }

        std::size_t GetOffsetForAttr(const std::string& id) {
            auto offset = attr_to_offset.find(id);
            if (offset != attr_to_offset.end()) return offset->second;
            return -1;
        }

        std::size_t GetOffsetForFormal(const std::string& id) {
            auto offset = arg_to_offset.find(id);
            if (offset != arg_to_offset.end()) return offset->second;
            return -1;
        }

        std::size_t GetOffsetForLocal(const std::string& id) {
            for (std::size_t i = local_to_offset.size(); i > 0; --i) {
                const auto& scope = local_to_offset[i - 1];
                auto offset = scope.find(id);
                if (offset != scope.end()) return offset->second;
            }
            return -1;
        }

    private:
        std::string class_name_;
        std::string method_name_;
        std::map<std::string, int> attr_to_offset;
        std::map<std::string, int> arg_to_offset;
        std::vector<std::map<std::string, std::size_t>> local_to_offset;
    };
}

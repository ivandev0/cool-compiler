#pragma once
namespace backend {
    class Context {
    public:
        explicit Context(const semant::TypeEnvironment &typeEnv) : type_env_(typeEnv) {}

        void EnterClass(const parser::Class& klass) {
            current_class_ = klass;
            AddAttrs(type_env_.class_table_.GetAllAttributesOf(klass.type));
        }

        void EnterMethod(const parser::MethodFeature& method) {
            method_name_ = method.id.id;
            arg_to_offset.clear();
            for (std::size_t i = 0; i < method.params.size(); ++i) {
                arg_to_offset[method.params[i].id.id] = 12 + 4 * (method.params.size() - 1) - 4 * i;
            }
        }

        void AddAttrs(const std::vector<parser::AttrFeature>& attrs) {
            attr_to_offset.clear();
            for (const auto& attr: attrs) {
                attr_to_offset[attr.id.id] = 12 + 4 * attr_to_offset.size();
            }
        }

        void AddLocalId(const std::string& id) {
            locals.push_back(id);
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
            for (std::size_t i = locals.size(); i > 0; --i) {
                if (locals[i] == id) {
                    return -4 * (locals.size() - 1 - i);
                }
            }
            return -1;
        }

        std::string GetSelfType() const {
            return current_class_.type;
        }

        std::string GetFileName() const {
            return current_class_.filename;
        }

    private:
        const semant::TypeEnvironment& type_env_;

        parser::Class current_class_;
        std::string method_name_;
        std::map<std::string, int> attr_to_offset;
        std::map<std::string, int> arg_to_offset;
        std::vector<std::string> locals;
    };
}

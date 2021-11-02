#pragma once
#include <map>
#include <vector>

namespace semant {
    struct Method {
        std::string class_name;
        std::string method_name;

        bool operator<(const Method& other) const {
            return std::tie(class_name, method_name) < std::tie(other.class_name, other.method_name);
        }
    };

    struct Signature {
        std::vector<std::string> types;
    };

    class MethodEnvironment {
    public:
        explicit MethodEnvironment(const std::vector<parser::Class>& classes, const ClassTable& class_table) {
            std::vector<parser::Class> all_classes = classes;
            all_classes.push_back(class_table.object_class_);
            all_classes.push_back(class_table.int_class_);
            all_classes.push_back(class_table.bool_class_);
            all_classes.push_back(class_table.str_class_);
            all_classes.push_back(class_table.io_class_);

            AddToMap(class_table.object_class_, class_table.object_class_.type, all_classes, method_to_signature_, class_table);
            AddToMap(class_table.int_class_, class_table.int_class_.type, all_classes, method_to_signature_, class_table);
            AddToMap(class_table.bool_class_, class_table.bool_class_.type, all_classes, method_to_signature_, class_table);
            AddToMap(class_table.str_class_, class_table.str_class_.type, all_classes, method_to_signature_, class_table);
            AddToMap(class_table.io_class_, class_table.io_class_.type, all_classes, method_to_signature_, class_table);
            for (const auto& class_ : classes) {
                AddToMap(class_, class_.type, all_classes, method_to_signature_, class_table);
            }
        }

        Signature* GetMethodSignature(const std::string& class_name, const std::string& method_name) {
            auto sig = method_to_signature_.find({class_name, method_name});
            if (sig == method_to_signature_.end()) return nullptr;
            return &sig->second;
        }

    private:
        static void AddToMap(
            const parser::Class& class_,
            const std::string& self_type,
            const std::vector<parser::Class>& all_classes,
            std::map<Method, Signature>& method_to_signature,
            const ClassTable& table
        ) {
            if (class_.type != table.object_class_.type) {
                auto predicate = [&class_](const parser::Class& c) { return class_.parent == c.type; };
                auto parent = std::find_if(all_classes.begin(), all_classes.end(), predicate);
                AddToMap(*parent, self_type, all_classes, method_to_signature, table);
            }

            for (auto feature : class_.features) {
                if (auto data = std::get_if<parser::MethodFeature>(&feature.feature)) {
                    CheckAllParamsAreDefined(*data, table);

                    auto method = method_to_signature.find({self_type, data->id.id});
                    if (method != method_to_signature.end()) {
                        CheckAllParamsMatchOriginal(*data, method->first, method->second);
                    }
                    method_to_signature[{self_type, data->id.id}] = ToSignature(*data);
                }
            }
        }

        static Signature ToSignature(const parser::MethodFeature& feature) {
            std::vector<std::string> params = FormalsToTypes(feature.params);
            params.push_back(feature.return_type);
            return {params};
        }

        static std::vector<std::string> FormalsToTypes(std::vector<parser::Formal> formals) {
            std::vector<std::string> result;
            std::transform(formals.begin(), formals.end(), std::back_inserter(result), [](const parser::Formal& formal) { return formal.type; });
            return result;
        }

        static void CheckAllParamsAreDefined(const parser::MethodFeature& feature, const ClassTable& table) {
            for (const auto& param: feature.params) {
                if (param.id.id == "self") {
                    throw std::runtime_error("'self' cannot be the name of a formal parameter.");
                }
                if (!table.IsDefined(param.type)) {
                    throw std::runtime_error("Class " + param.type + " of formal parameter " + param.id.id + " is undefined.");
                }
            }
            if (!table.IsDefined(feature.return_type)) {
                throw std::runtime_error("Undefined return type " + feature.return_type + " in method " + feature.id.id + ".");
            }
        }

        static void CheckAllParamsMatchOriginal(const parser::MethodFeature& feature, const Method& original_method, const Signature& sig) {
            if (sig.types.size() - 1 != feature.params.size()) {
                throw std::runtime_error("Incompatible number of formal parameters in redefined method " + original_method.method_name + ".");
            }
            for (std::size_t i = 0; i < sig.types.size(); ++i) {
                auto original = sig.types[i];
                auto param = i != sig.types.size() - 1 ? feature.params[i].type : feature.return_type;
                auto param_message = i != sig.types.size() - 1 ? "parameter type " : "return type ";
                auto type_message = i != sig.types.size() - 1 ? "type " : "return type ";

                if (param != original) {
                    std::string message;
                    message.append("In redefined method ").append(original_method.method_name).append(", ")
                            .append(param_message).append(param).append(" is different from original ")
                            .append(type_message).append(original).append(".");
                    throw std::runtime_error(message);
                }
            }
        }
    private:
        std::map<Method, Signature> method_to_signature_;
    };
}

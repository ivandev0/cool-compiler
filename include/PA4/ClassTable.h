#pragma once
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "Parser.h"

namespace semant {
    struct Node {
        std::string name;
        std::string parent;
        std::vector<std::shared_ptr<Node>> children;
    };

    class ClassTable {
    public:
        explicit ClassTable(const std::vector<parser::Class>& classes): classes_(classes) {
            for (const auto& node : basic_classes_) {
                graph_[node.name] = std::make_shared<Node>(node);
            }

            for (const auto& class_ : classes) {
                if (IsBasicClass(class_.type) || class_.type == self_type) {
                    throw std::runtime_error("Redefinition of basic class " + class_.type + ".");
                }

                if (graph_.find(class_.type) != graph_.end()) {
                    throw std::runtime_error("Class " + class_.type + " was previously defined.");
                }

                graph_[class_.type] = std::make_shared<Node>(Node{class_.type, class_.parent, {}});
            }

            for (const auto& class_ : classes) {
                auto node = graph_[class_.type];

                if (node->parent == self_type || IsBasicPrimitiveClass(node->parent)) {
                    throw std::runtime_error("Class " + class_.type + " cannot inherit class " + node->parent + ".");
                }

                auto parentNode = graph_.find(node->parent);
                if (parentNode == graph_.end()) {
                    throw std::runtime_error("Class " + class_.type + " inherits from an undefined class " + node->parent + ".");
                }
                parentNode->second->children.push_back(node);
            }

            for (std::size_t i = classes_.size() - 1; i > 0; --i) {
                auto node = graph_.at(classes_[i].type);
                std::set<std::string> visited{node->name};
                CheckForCyclicInheritance(node, visited);
            }
        }

        bool CheckAIsSubtypeOfB(const std::string& a, const std::string& b) const {
            if (b == object_.name) return true;

            auto node = graph_.at(a);
            while (node->name != object_.name) {
                if (node->name == b) return true;
                node = graph_.at(node->parent);
            }
            return false;
        }

        bool IsDefined(const std::string& type) const {
            if (type == self_type) return true;
            return graph_.find(type) != graph_.end();
        }

        bool IsBasicPrimitiveClass(const std::string& name) const {
            return name == int_class_.type || name == bool_class_.type || name == str_class_.type;
        }

        bool IsBasicClass(const std::string& name) const {
            auto predicate = [&name](const Node &n) { return n.name == name; };
            return std::find_if(basic_classes_.begin(), basic_classes_.end(), predicate) != basic_classes_.end();
        }

        std::string CommonSuperType(const std::string& a, const std::string& b) const {
            auto node1 = graph_.at(a);
            auto node2 = graph_.at(b);

            std::vector<std::string> inheritance1;
            while (node1->name != object_.name) {
                inheritance1.push_back(node1->name);
                node1 = graph_.at(node1->parent);
            }
            std::reverse(inheritance1.begin(), inheritance1.end());

            std::vector<std::string> inheritance2;
            while (node2->name != object_.name) {
                inheritance2.push_back(node2->name);
                node2 = graph_.at(node2->parent);
            }
            std::reverse(inheritance2.begin(), inheritance2.end());

            auto last_common = object_class_.type;
            for (std::size_t i = 0; i < std::min(inheritance1.size(), inheritance2.size()); ++i) {
                if (inheritance1[i] != inheritance2[i]) return last_common;
                last_common = inheritance1[i];
            }

            return last_common;
        }

        parser::Class GetClassByName(const std::string& type) const {
            if (type == object_class_.type) return object_class_;
            if (type == io_class_.type) return io_class_;
            if (type == int_class_.type) return int_class_;
            if (type == str_class_.type) return str_class_;
            if (type == bool_class_.type) return bool_class_;
            return *std::find_if(classes_.begin(), classes_.end(), [&type](const parser::Class& klass) { return type == klass.type; });
        }

        std::vector<parser::AttrFeature> GetAttributesOf(const std::string& type) const {
            if (type == object_class_.type || type == io_class_.type) return {};
            parser::Class klass = GetClassByName(type);

            std::vector<parser::AttrFeature> attrs;
            for (auto feature : klass.features) {
                if (auto data = std::get_if<parser::AttrFeature>(&feature.feature)) {
                    attrs.push_back(*data);
                }
            }
            return attrs;
        }

        std::vector<parser::AttrFeature> GetAllAttributesOf(const std::string& type) const {
            if (type == object_class_.type || type == io_class_.type) return {};
            parser::Class klass = GetClassByName(type);

            std::vector<parser::AttrFeature> super_attrs = GetAllAttributesOf(klass.parent);
            std::vector<parser::AttrFeature> attrs = GetAttributesOf(type);
            super_attrs.insert(super_attrs.end(), attrs.begin(), attrs.end());

            return super_attrs;
        }

        std::string GetParentOf(const std::string& type) const {
            auto node = graph_.at(type);
            return node->parent;
        }

        std::vector<parser::Class> GetDefinedClasses() const {
            std::vector<parser::Class> result;
            CollectDFS(object_class_, result);
            return result;
        }
    private:
        void CollectDFS(const parser::Class &klass, std::vector<parser::Class>& classes) const {
            auto node = graph_.at(klass.type);
            classes.push_back(GetClassByName(node->name));
            for (const auto& child: node->children) {
                CollectDFS(GetClassByName(child->name), classes);
            }
        }

        void CheckForCyclicInheritance(const std::shared_ptr<Node>& node, std::set<std::string>& visited) {
            for (const auto& child : node->children) {
                auto result = visited.insert(child->name);
                if (!result.second) {
                    throw std::runtime_error("Class " + child->name + ", or an ancestor of " + child->name + ", is involved in an inheritance cycle.");
                }
                CheckForCyclicInheritance(child, visited);
                visited.erase(result.first);
            }
        }

        static parser::Class InitObject() {
            auto abort = parser::MethodFeature{{}, {{}, "abort"}, {}, "Object", NoExpr()};
            auto type_name = parser::MethodFeature{{}, {{}, "type_name"}, {}, "String", NoExpr()};
            auto copy = parser::MethodFeature{{}, {{}, "copy"}, {}, self_type, NoExpr()};

            return {
                {}, "Object", "Object",
                {parser::Feature{{}, abort}, parser::Feature{{}, type_name}, parser::Feature{{}, copy}},
                "<basic class>"
            };
        }

        static parser::Class InitInt() {
            auto val_attr = parser::AttrFeature{{}, {{}, "val"}, "Int", NoExpr()};
            return {{}, "Int", "Object", {parser::Feature{{}, val_attr}}, "<basic class>"};
        }

        static parser::Class InitBool() {
            auto val_attr = parser::AttrFeature{{}, {{}, "val"}, "Bool", NoExpr()};
            return {{}, "Bool", "Object", {parser::Feature{{}, val_attr}}, "<basic class>"};
        }

        static parser::Class InitString() {
            auto val_attr = parser::AttrFeature{{}, {{}, "val"}, "Int", NoExpr()};
            auto str_attr = parser::AttrFeature{{}, {{}, "str_field"}, "String", NoExpr()};
            auto length = parser::MethodFeature{{}, {{}, "length"}, {}, "Int", NoExpr()};
            auto concat = parser::MethodFeature{
                {}, {{}, "concat"},
                {parser::Formal{{}, {{}, "s"}, "String"}},
                "String", NoExpr()
            };
            auto substr = parser::MethodFeature{
                {},
                {{}, "substr"},
                {parser::Formal{{}, {{}, "i"}, "Int"}, parser::Formal{{}, {{}, "l"}, "Int"}},
                "String", NoExpr()
            };

            return {
                {}, "String", "Object",
                {
                    parser::Feature{{}, val_attr},
                    parser::Feature{{}, str_attr},
                    parser::Feature{{}, length},
                    parser::Feature{{}, concat},
                    parser::Feature{{}, substr}
                },
                "<basic class>"
            };
        }

        static parser::Class InitIO() {
            auto out_string = parser::MethodFeature{
                    {}, {{}, "out_string"},
                    {parser::Formal{{}, {{}, "x"}, "String"}},
                    self_type, NoExpr()
            };
            auto out_int = parser::MethodFeature{
                    {}, {{}, "out_int"},
                    {parser::Formal{{}, {{}, "x"}, "Int"}},
                    self_type, NoExpr()
            };
            auto in_string = parser::MethodFeature{{}, {{}, "in_string"}, {}, "String", NoExpr()};
            auto in_int = parser::MethodFeature{{}, {{}, "in_int"}, {}, "Int", NoExpr()};

            return {
                {}, "IO", "Object",
                {
                    parser::Feature{{}, out_string},
                    parser::Feature{{}, out_int},
                    parser::Feature{{}, in_string},
                    parser::Feature{{}, in_int},
                },
                "<basic class>"
            };
        }

        static std::shared_ptr<parser::Expression> NoExpr() {
            return std::make_shared<parser::Expression>(parser::Parser::CreateNoExpr());
        }

    public:
        inline static std::string self_type = "SELF_TYPE";

        parser::Class io_class_ = InitIO();
        parser::Class int_class_ = InitInt();
        parser::Class bool_class_ = InitBool();
        parser::Class str_class_ = InitString();
        parser::Class object_class_ = InitObject();

    private:
        Node io_node_{"IO", "Object", {}};
        Node int_node_{"Int", "Object", {}};
        Node bool_node_{"Bool", "Object", {}};
        Node string_node_{"String", "Object", {}};
        Node object_{"Object", "Object", {std::make_shared<Node>(io_node_), std::make_shared<Node>(int_node_), std::make_shared<Node>(bool_node_), std::make_shared<Node>(string_node_)}};
        std::vector<Node> basic_classes_{object_, io_node_, int_node_, bool_node_, string_node_};

        std::map<std::string, std::shared_ptr<Node>> graph_;
        std::vector<parser::Class> classes_;
    };
}

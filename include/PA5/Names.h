#pragma once

namespace backend {
    class Names {
    public:
    public:
        inline static std::string obj_name = "Object";
        inline static std::string io_name = "IO";
        inline static std::string int_name = "Int";
        inline static std::string str_name = "String";
        inline static std::string bool_name = "Bool";

        inline static std::string prot_suffix = "_protObj";
        inline static std::string disp_suffix = "_dispTab";
        inline static std::string init_suffix = "_init";
        inline static std::string heap_start = "heap_start";
        inline static std::string main = "Main";
        inline static std::string name_tab = "class_nameTab";
        inline static std::string obj_tab = "class_objTab";

        inline static std::string main_call = main + ".main";
        inline static std::string dispatch_abort = "_dispatch_abort";
        inline static std::string case_no_match = "case_abort";
        inline static std::string case_on_void = "case_abort2";
        inline static std::string equality_test = "equality_test";
        inline static std::string copy = "Object.copy";

    public:
        static std::string FormProtObjName(const std::string& type) { return type + Names::prot_suffix; }
        static std::string FormDispTableName(const std::string& type) { return type + Names::disp_suffix; }
        static std::string FormInit(const std::string& type) { return type + Names::init_suffix; }

    };
}
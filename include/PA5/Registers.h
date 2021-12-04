#pragma once
#include <utility>

namespace backend {
    struct Reg {
    public:
        explicit Reg(std::string name) : name(std::move(name)) {}
        std::string ToString() const { return name; }
        Reg Shift(int i) const { return Reg(std::to_string(i) + "(" + name + ")"); }

        friend std::ostream& operator<<(std::ostream& os, const Reg& reg) {
            os << reg.ToString();
            return os;
        }
    private:
        const std::string name;
    };

    class R {
    public:
        inline static Reg acc{"$a0"}, a1{"$a1"}, fp{"$fp"}, sp{"$sp"}, ra{"$ra"}, zero{"$zero"}, s0{"$s0"};
        inline static Reg t0{"$t0"}, t1{"$t1"}, t2{"$t2"}, t3{"$t3"}, t4{"$t4"}, t5{"$t5"}, t6{"$t6"}, t7{"$t7"}, t8{"$t8"};
    };
}
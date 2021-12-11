#include <criterion/criterion.h>

class addition {
public:
    int add(int a, int b) {
        return a + b;
    }
};

Test(hello, world) {
    cr_assert_eq(addition().add(1,1), 2);
}
#include <cstdio>
#include <dotenv.hpp>

int main() {
    dotenv::load();

    printf("TEST_VAR=%s\n", dotenv::get("TEST_VAR").data());
}

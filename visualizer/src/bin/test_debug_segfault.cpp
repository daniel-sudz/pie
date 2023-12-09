#include <csignal>

/* Test file to make sure we get useful traces on a segfault */
int main() {
    std::raise(SIGSEGV);
}

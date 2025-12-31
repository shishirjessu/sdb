#include <cstdio>
#include <iostream>
#include <memory>
#include <sys/signal.h>
#include <unistd.h>

int main() {
    // for read test
    unsigned long value = 0xcafecafe;
    auto addr = std::addressof(value);

    write(STDOUT_FILENO, std::addressof(addr), sizeof(void*));
    fflush(stdout);
    raise(SIGTRAP);

    // for write test
    char toWrite[12] = {0};
    auto writeAddr = std::addressof(toWrite);
    write(STDOUT_FILENO, std::addressof(writeAddr), sizeof(void*));
    fflush(stdout);
    raise(SIGTRAP);

    printf("%s", toWrite);
}
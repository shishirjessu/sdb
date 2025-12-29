int main() {
    [[maybe_unused]] volatile int i;
    while (true)
        i = 42;
}
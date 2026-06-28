extern int printf(const char *fmt, ...);

int main(void) {
    int i = 0;
    while (i < 5) {
        printf("%d\n", i);
        i = i + 1;
    }
    i = 3;
    while (i > 0) {
        printf("%d\n", i);
        i = i - 1;
    }
    return 0;
}

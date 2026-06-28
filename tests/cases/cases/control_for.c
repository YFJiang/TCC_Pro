extern int printf(const char *fmt, ...);

int main(void) {
    int i;
    for (i = 0; i < 5; i = i + 1) {
        printf("%d\n", i);
    }
    for (i = 5; i > 0; i = i - 1) {
        printf("%d\n", i);
    }
    return 0;
}

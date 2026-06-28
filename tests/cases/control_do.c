extern int printf(const char *fmt, ...);

int main(void) {
    int i = 0;
    do {
        printf("%d\n", i);
        i = i + 1;
    } while (i < 3);
    return 0;
}

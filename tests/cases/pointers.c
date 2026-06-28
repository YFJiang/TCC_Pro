extern int printf(const char *fmt, ...);

int main(void) {
    int x = 42;
    int *p = &x;
    printf("%d\n", *p);
    *p = 99;
    printf("%d\n", x);
    return 0;
}

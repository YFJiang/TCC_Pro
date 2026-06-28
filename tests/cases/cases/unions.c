extern int printf(const char *fmt, ...);

union value {
    int i;
    float f;
};

int main(void) {
    union value v;
    v.i = 42;
    printf("%d\n", v.i);
    return 0;
}

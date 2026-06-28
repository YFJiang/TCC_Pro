extern int printf(const char *fmt, ...);

int add(int a, int b) {
    return a + b;
}

int mul(int a, int b) {
    return a * b;
}

int main(void) {
    int x = add(3, 4);
    int y = mul(x, 2);
    printf("%d\n", x);
    printf("%d\n", y);
    return 0;
}

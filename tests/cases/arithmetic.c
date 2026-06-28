extern int printf(const char *fmt, ...);

int main(void) {
    int a = 10, b = 3;
    printf("%d %d %d %d %d\n", a + b, a - b, a * b, a / b, a % b);
    printf("%d %d %d\n", (a > b), (a < b), (a == b));
    printf("%d %d %d\n", (a >= b), (a <= b), (a != b));
    printf("%d %d\n", a & b, a | b);
    return 42;
}

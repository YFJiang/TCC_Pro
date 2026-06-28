extern int printf(const char *fmt, ...);

typedef int my_int;
typedef int my_arr[3];

int main(void) {
    my_int x = 42;
    my_arr a;
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    printf("%d\n", x);
    printf("%d\n", a[0] + a[1] + a[2]);
    return 0;
}

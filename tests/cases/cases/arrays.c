extern int printf(const char *fmt, ...);

int main(void) {
    int arr[5];
    int i;
    for (i = 0; i < 5; i = i + 1) {
        arr[i] = i * 10;
    }
    for (i = 0; i < 5; i = i + 1) {
        printf("%d\n", arr[i]);
    }
    printf("%d\n", arr[2] + arr[4]);
    return 0;
}

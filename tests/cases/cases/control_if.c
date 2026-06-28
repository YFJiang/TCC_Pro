extern int printf(const char *fmt, ...);

int main(void) {
    int x = 5;
    if (x > 3) {
        printf("greater\n");
    } else {
        printf("not greater\n");
    }
    if (x == 0) {
        printf("zero\n");
    } else if (x == 5) {
        printf("five\n");
    }
    return 0;
}

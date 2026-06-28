extern int printf(const char *fmt, ...);

int main(void) {
    int i = 0;
start:
    if (i < 3) {
        printf("%d\n", i);
        i = i + 1;
        goto start;
    }
    printf("done\n");
    return 0;
}

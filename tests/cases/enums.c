extern int printf(const char *fmt, ...);

enum color { RED, GREEN, BLUE };

int main(void) {
    enum color c = GREEN;
    printf("%d\n", c);
    printf("%d %d %d\n", RED, GREEN, BLUE);
    return 0;
}

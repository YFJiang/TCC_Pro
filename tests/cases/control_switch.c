extern int printf(const char *fmt, ...);

int main(void) {
    int x = 2;
    switch (x) {
    case 1:  printf("one\n");  break;
    case 2:  printf("two\n");  break;
    case 3:  printf("three\n"); break;
    default: printf("other\n"); break;
    }
    switch (x) {
    case 1: printf("1\n");
    case 2: printf("2\n");
    case 3: printf("3\n"); break;
    default: printf("d\n");
    }
    return 0;
}

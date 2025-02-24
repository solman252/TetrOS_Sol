static void clear_screen(void) {
    volatile unsigned short *video = (volatile unsigned short *)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        video[i] = 0x0720;
    }
}

void kernel_main(void) {
    clear_screen();

    volatile unsigned short *video = (volatile unsigned short *)0xB8000;
    const char *message = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    int i = 0;

    while (message[i] != '\0') {
        video[i] = (0x07 << 8) | message[i];
        i++;
    }

    while (1) { }
}

void _start(void) {
    kernel_main();
}

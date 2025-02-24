void kernel_main(void) {
    // vga text buffer starts at 0xB8000.
    volatile char *video = (volatile char *)0xB8000;
    const char *message = "Hello from kernel!";

    // print the message.
    for (int i = 0; message[i] != '\0'; i++) {
        video[i * 2]     = message[i];
        video[i * 2 + 1] = 0x07;
    }

    // halt cpu forever
    while (1);
}

void _start(void) {
    kernel_main();
}

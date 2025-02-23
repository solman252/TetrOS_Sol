// kernel.c
void kernel_main() {
    char *video_memory = (char *)0xB8000; // VGA text buffer
    const char *message = "Welcome to tetrOS!";
    
    for (int i = 0; message[i] != '\0'; i++) {
        video_memory[i * 2] = message[i]; // Character
        video_memory[i * 2 + 1] = 0x07;   // Color attribute (white on black)
    }

    while (1); // Halt
}

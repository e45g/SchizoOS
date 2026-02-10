#include <common.h>
#include <video.h>

void kmain(boot_info_t *boot_info) {
    draw_string(boot_info, "Welcome to SchizoOS", 0, 0, 0x00FF0000);

    for (;;) __asm__ volatile ("hlt");
}

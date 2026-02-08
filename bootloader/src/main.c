#include <efi.h>
#include <efilib.h>


EFI_STATUS
EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"[Boot] UEFI Bootloader starting...\r\n");

halt:
    for(;;) __asm__ volatile("hlt");

    return EFI_SUCCESS;
}

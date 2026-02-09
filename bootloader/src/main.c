#include "elf.h"
#include <efi.h>
#include <efilib.h>


EFI_STATUS
EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    InitializeLib(ImageHandle, SystemTable);
    Print(L"[Boot] UEFI Bootloader starting...\r\n");

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    // Get GOP
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void **)&gop);
    if (EFI_ERROR(status)) {
        Print(L"[Error] Could not locate GOP: %r\r\n", status);
        goto halt;
    }
    Print(L"[Ok] GOP: %dx%d, FB at 0x%lx\r\n",
          gop->Mode->Info->HorizontalResolution,
          gop->Mode->Info->VerticalResolution,
          gop->Mode->FrameBufferBase);


    // Load kernel
    EFI_LOADED_IMAGE_PROTOCOL *loaded_img;
    EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    status = uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &loaded_image_guid, (void **)&loaded_img);
    if(EFI_ERROR(status)) {
        Print(L"[Error] Could not get loaded image protocol\r\n");
        goto halt;
    }


    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    status = uefi_call_wrapper(BS->HandleProtocol, 3, loaded_img->DeviceHandle, &fs_guid, (void **)&fs);
    if(EFI_ERROR(status)) {
        Print(L"[Error] Could not get filesystem protocol\r\n");
        goto halt;
    }

    EFI_FILE_HANDLE root;
    status = uefi_call_wrapper(fs->OpenVolume, 2, fs, &root);
    if(EFI_ERROR(status)) {
        Print(L"[Error] Could not open root volume\r\n");
        goto halt;
    }

    EFI_FILE_HANDLE kernel_file;
    status = uefi_call_wrapper(root->Open, 5, root, &kernel_file, L"\\kernel.elf", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if(EFI_ERROR(status)) {
        Print(L"[Error] Could not open kernel.elf\r\n");
        goto halt;
    }
    Print(L"[Ok] Kernel file opened\r\n");

    EFI_FILE_INFO *file_info = NULL;
    UINTN info_size = 0;
    EFI_GUID file_info_guid = EFI_FILE_INFO_ID;
    status = uefi_call_wrapper(kernel_file->GetInfo, 4, kernel_file, &file_info_guid, &info_size, file_info);
    if(status == EFI_BUFFER_TOO_SMALL) {
        status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, info_size, (void **)&file_info);
        if (EFI_ERROR(status)) {
            Print(L"[Error] Could not allocate for file info\r\n");
            goto halt;
        }

        status = uefi_call_wrapper(kernel_file->GetInfo, 4, kernel_file, &file_info_guid, &info_size, file_info);
        if (EFI_ERROR(status)) {
            Print(L"[Error] Could not get kernel file info\r\n");
            goto halt;
        }
    }
    else if (EFI_ERROR(status)) {
        Print(L"[Error] Could not get info size\r\n");
        goto halt;
    }

    UINTN kernel_file_size = file_info->FileSize;
    Print(L"[Ok] Kernel size: %d bytes\r\n", kernel_file_size);

    UINT8 *kernel_buffer;
    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, kernel_file_size, (void **)&kernel_buffer);
    if (EFI_ERROR(status)) {
        Print(L"[Error] Could not allocate for kernel buffer\r\n");
        goto halt;
    }

    status = uefi_call_wrapper(kernel_file->Read, 3, kernel_file, kernel_file_size, kernel_buffer);
    if (EFI_ERROR(status)) {
        Print(L"[Error] Could not read kernel file\r\n");
        goto halt;
    }
    Print(L"[Ok] Kernel loaded into buffer\r\n");

    // parse kernel
    elf64_header_t *elf = (elf64_header_t*)kernel_buffer;
    if(elf->magic != ELF_MAGIC) {
        Print(L"[Error] Invalid ELF magic\r\n");
        goto halt;
    }
    Print(L"[Ok] Valid ELF64, entry: 0x%lx\r\n", elf->entry);


halt:
    for(;;) __asm__ volatile("hlt");

    return EFI_SUCCESS;
}

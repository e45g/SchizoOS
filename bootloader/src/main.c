#include "elf.h"
#include "x86_64/efibind.h"
#include <efi.h>
#include <efilib.h>

typedef struct {
    void* base_address;
    uint64_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
} framebuffer_t;

typedef struct {
    void* map_begin;
    uint64_t map_size;
    uint64_t descriptor_size;
} memory_map_t;

typedef struct {
    framebuffer_t framebuffer;
    memory_map_t  memory_map;
    void* rsdp;
} boot_info_t;

typedef void (*kernel_entry_t)(boot_info_t *);

EFI_STATUS
EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    boot_info_t boot_info = {0};
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
    boot_info.framebuffer.base_address = (void*)gop->Mode->FrameBufferBase;
    boot_info.framebuffer.buffer_size = gop->Mode->FrameBufferSize;
    boot_info.framebuffer.width = gop->Mode->Info->HorizontalResolution;
    boot_info.framebuffer.height = gop->Mode->Info->VerticalResolution;
    boot_info.framebuffer.pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

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
    status = uefi_call_wrapper(root->Open, 5, root, &kernel_file, L"\\kernel.elf", EFI_FILE_MODE_READ, 0);
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
        Print(L"[Error] Could not allocate memory for kernel buffer\r\n");
        goto halt;
    }

    UINTN read_size = kernel_file_size;
    status = uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &read_size, kernel_buffer);
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

    elf64_phdr_t *phdr = (elf64_phdr_t *)(kernel_buffer + elf->phoff);
    UINTN v_mem_start = INT64_MAX;
    UINTN v_mem_end = 0;
    UINTN p_mem_start = INT64_MAX;
    UINTN p_mem_end = 0;
    for(INT32 i = 0; i < elf->phnum; i++, phdr++) {
        if(phdr->type == PT_LOAD) {
            if(phdr->vaddr < v_mem_start) v_mem_start = phdr->vaddr;
            if(phdr->vaddr > v_mem_end) v_mem_end = phdr->vaddr;

            if(phdr->paddr < p_mem_start) p_mem_start = phdr->paddr;
            if(phdr->paddr > p_mem_end) p_mem_end = phdr->paddr;
        }
    }
    UINTN memory_needed = p_mem_end - p_mem_start;
    Print(L"[Ok] Memory needed by program headers: %ld\r\n", memory_needed);

    EFI_PHYSICAL_ADDRESS kernel_load_address = p_mem_start;
    UINTN pages_needed = (memory_needed + 0xFFF) / 0x1000;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages_needed, &kernel_load_address);
    if (EFI_ERROR(status)) {
        Print(L"[Error] Could not allocate memory for program buffer at 0x%lx: %r\r\n", p_mem_start, status);
        goto halt;
    }

    phdr = (elf64_phdr_t *)(kernel_buffer + elf->phoff);
    for(INT32 i = 0; i < elf->phnum; i++, phdr++) {
        if(phdr->type == PT_LOAD) {
            UINT8 *src = kernel_buffer + phdr->offset;
            UINT8 *dst = (UINT8 *)phdr->paddr;
            UINT64 len = phdr->filesz;

            status = uefi_call_wrapper(BS->CopyMem, 3, dst, src, len);
            if (EFI_ERROR(status)) {
                Print(L"[Error] Could not copy segment: %r\r\n", status);
                goto halt;
            }

            if(phdr->memsz > phdr->filesz) {
                status = uefi_call_wrapper(BS->SetMem, 3, dst + len, phdr->memsz - phdr->filesz, 0);
                if (EFI_ERROR(status)) {
                    Print(L"[Error] Could not zero BSS: %r\r\n", status);
                    goto halt;
                }
            }

            Print(L"[Ok] Loaded segment to 0x%lx, size: %ld\n", dst, len);
        }
    }

    Print(L"[Ok] Program loaded to 0x%lx, entry: 0x%lx\n", p_mem_start, elf->entry);


    UINTN map_size = 0;
    EFI_MEMORY_DESCRIPTOR *mem_map = NULL;
    UINTN map_key;
    UINTN desc_size;
    UINT32 desc_version;
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, mem_map, &map_key, &desc_size, &desc_version);
    if(!(status == EFI_BUFFER_TOO_SMALL || status == EFI_SUCCESS)) {
        Print(L"[Error] Could not get memory map (before alloc)\r\n");
        goto halt;
    }

    // If the MemoryMap buffer is too small, the EFI_BUFFER_TOO_SMALL error code is returned and the MemoryMap-
    // Size value contains the size of the buffer needed to contain the current memory map. The actual size of the buffer
    // allocated for the consequent call to GetMemoryMap() should be bigger then the value returned in MemoryMapSize,
    // since allocation of the new buffer may potentially increase memory map size.
    map_size += 4096;
    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, map_size, (void **)&mem_map);
    if (EFI_ERROR(status)) {
        Print(L"[Error] Could not allocate memory for memory map\r\n");
        goto halt;
    }

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, mem_map, &map_key, &desc_size, &desc_version);
    if(EFI_ERROR(status)) {
        Print(L"[Error] Could not get memory map\r\n");
        goto halt;
    }
    boot_info.memory_map.map_size = map_size;
    boot_info.memory_map.descriptor_size = desc_size;
    boot_info.memory_map.map_begin = mem_map;

    Print(L"[Ok] Jumping to kernel...\r\n");

    status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, map_key);
    if (EFI_ERROR(status)) {
        // Memory map changed, try again
        map_size = 0;
        uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, NULL, &map_key, &desc_size, &desc_version);
        map_size += 4096;
        uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, map_size, (void **)&mem_map);
        uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, mem_map, &map_key, &desc_size, &desc_version);

        boot_info.memory_map.map_size = map_size;
        boot_info.memory_map.descriptor_size = desc_size;
        boot_info.memory_map.map_begin = mem_map;
        status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, map_key);
        if (EFI_ERROR(status)) {
            goto halt;
        }
    }

    kernel_entry_t kernel_entry = (kernel_entry_t)elf->entry;
    kernel_entry(&boot_info);

halt:
    for(;;) __asm__ volatile("hlt");

    return EFI_SUCCESS;
}

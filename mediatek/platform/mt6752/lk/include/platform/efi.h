#ifndef __EFI_H__
#define __EFI_H__


typedef struct {
    u8 b[16];
} __attribute__((packed)) efi_guid_t;


typedef struct {
    u64 signature;
    u32 revision;
    u32 header_size;
    u32 header_crc32;
    u32 reserved;
    u64 my_lba;
    u64 alternate_lba;
    u64 first_usable_lba;
    u64 last_usable_lba;
    efi_guid_t disk_guid;
    u64 partition_entry_lba;
    u32 num_partition_entries;
    u32 sizeof_partition_entry;
    u32 partition_entry_array_crc32;
} __attribute__((packed)) gpt_header;


#define GPT_ENTRY_NAME_LEN  (72 / sizeof(u16))

typedef struct {
    efi_guid_t partition_type_guid;
    efi_guid_t unique_partition_guid;
    u64 starting_lba;
    u64 ending_lba;
    u64 attributes;
    u16 partition_name[GPT_ENTRY_NAME_LEN];
} __attribute__((packed))gpt_entry;


#define GPT_HEADER_SIGNATURE    0x5452415020494645ULL

#endif

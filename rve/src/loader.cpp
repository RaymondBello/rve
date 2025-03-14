
#include "loader.h"


int loadElf(const char *path, uint64_t path_len, uint8_t *data, uint64_t data_len)
{

    // Open in binary mode
    uint32_t fd = open(path, O_RDONLY | O_SYNC);
    if (fd != 3) {
        printf("ERRO: Failed to open ELF file\n");
        return 1;
    }
    printf("INFO: Opened ELF binary file: %s\n", path);

    /* ELF header : at start of file */
    Elf32_Ehdr eh;

    assert(&eh != NULL);
    assert(lseek(fd, (off_t)0, SEEK_SET) == (off_t)0);
    assert(read(fd, (void *)&eh, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr));
    printf("INFO: %s Read %0ld bytes of ELF32 Header\n", __func__, sizeof(Elf32_Ehdr));

    if (!strncmp((char *)eh.e_ident, "\177ELF", 4))
    {
        printf("INFO: %s ELFMAGIC = ELF\n", __func__);
        /* IS a ELF file */
    }
    else
    {
        printf("ERRO: %s ELFMAGIC mismatch!\n", __func__);
        /* Not ELF file */
        return 2;
    }

    if (eh.e_ident[EI_CLASS] == ELFCLASS64)
    {
        printf("ERRO: %s 64b ELF. Currently unsupported...\n", __func__);
        return 3;
    }
    else if (eh.e_ident[EI_CLASS] == ELFCLASS32)
    {
        printf("INFO: %s 32b ELF\n", __func__);

        // *section - header table is variable size * /
        Elf32_Shdr *sh_tbl;
        /* Section header table :  */
        sh_tbl = (Elf32_Shdr*) malloc(eh.e_shentsize * eh.e_shnum);
        if (!sh_tbl)
        {
            printf("ERRO: %s Failed to allocate %d bytes\n", __func__,
                   (eh.e_shentsize * eh.e_shnum));
        }
        // Read section header table
        uint32_t i;

        struct ElfSection
        {
            uint32_t addr_real;
            uint32_t offset;
            uint32_t size;
            uint8_t* sData;
        };

        std::vector<ElfSection> sections;

        assert(lseek(fd, (off_t)eh.e_shoff, SEEK_SET)== (off_t)eh.e_shoff);
        for (i = 0; i < eh.e_shnum; i++) 
        {

            ElfSection section;

            assert(read(fd, (void *)&sh_tbl[i], eh.e_shentsize) == eh.e_shentsize);
            if (sh_tbl[i].sh_type == SHT_PROGBITS)
            {
                section.addr_real = sh_tbl[i].sh_addr & 0x7FFFFFFF;
                section.size = sh_tbl[i].sh_size;
                section.offset = sh_tbl[i].sh_offset;
                sections.push_back(section);
            }
        }

        // Load section data
        for (int i = 0; i < (int)sections.size(); i++)
        {
            uint32_t off = sections[i].offset;
            uint32_t size = sections[i].size;
            uint32_t addr = sections[i].addr_real;
            printf("INFO: %s Section %0d size: %0d bytes\n", __func__, i, size);
            printf("INFO: %s Section %0d offset: 0x%0x\n", __func__, i, off);
            printf("INFO: %s Section %0d addr: 0x%0x\n", __func__, i, addr);
            assert(lseek(fd, (off_t)off, SEEK_SET) == (off_t)off);
            sections[i].sData = (uint8_t *)malloc( size );
            assert(read(fd, sections[i].sData, size) == size);

            // Write to data
            if (addr + size > data_len)
            {
                printf("ERRO: %s ELF section too big or offset too great\n", __func__);
                return 4;
            }
            for (uint32_t byte = 0; byte < size; byte++)
            {
                data[addr + byte] = sections[i].sData[byte];
                // printf("%0x\n", sections[i].sData[byte]);
            }
        }
        
        printf("INFO: Loaded Elf into memory\n");
    }

    close(fd);
    return 0;
}

int loadBinary(const char *path, uint64_t path_len, uint8_t *data, uint64_t data_len)
{
    // Ensure the path is null-terminated
    std::string filepath(path, path_len);

    // Print the path of the binary file being loaded
    std::cout << "Loading binary file '" << filepath << "'" << std::endl;

    // Open the binary file
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Failed to open binary file: " << filepath << std::endl;
        throw std::runtime_error("File open failed");
    }

    // Get file size
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Check if the file size exceeds the provided buffer size
    if (static_cast<uint64_t>(file_size) > data_len)
    {
        std::cerr << "Binary file too large for provided buffer" << std::endl;
        throw std::runtime_error("Buffer too small");
    }

    // Read the file content into the provided buffer
    if (!file.read(reinterpret_cast<char *>(data), file_size))
    {
        std::cerr << "Failed to read binary file" << std::endl;
        throw std::runtime_error("File read failed");
    }

    // Report success
    std::cout << "Successfully loaded binary file, size: " << file_size << " bytes" << std::endl;

    return 0;
}

#include "loader.h"

int loadElf(const char *path, uint64_t path_len, uint8_t *data, uint64_t data_len)
{
    // Print the path of the ELF binary being loaded
    std::cout << "Loading ELF binary '" << path << "'" << std::endl;

    // Open the ELF file
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        std::cerr << "Failed to open ELF file: " << strerror(errno) << std::endl;
        return 1;
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        std::cerr << "Failed to get file stats: " << strerror(errno) << std::endl;
        close(fd);
        return 2;
    }

    // Map the file into memory
    void *elf_data = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (elf_data == MAP_FAILED)
    {
        std::cerr << "Failed to map ELF file: " << strerror(errno) << std::endl;
        close(fd);
        return 3;
    }

    // Parse ELF header
    auto *ehdr = static_cast<Elf64_Ehdr *>(elf_data);

    // Check if it's a valid ELF file
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Not a valid ELF file" << std::endl;
        munmap(elf_data, st.st_size);
        close(fd);
        return 4;
    }

    // Get section header table
    auto *shdr = static_cast<Elf64_Shdr *>(
        static_cast<void *>(static_cast<uint8_t *>(elf_data) + ehdr->e_shoff));

    // Get section header string table
    Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
    const char *strtab = static_cast<const char *>(elf_data) + sh_strtab->sh_offset;

    // Iterate through each section
    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        Elf64_Shdr *section = &shdr[i];

        // Skip sections that are not of type SHT_PROGBITS (1)
        if (section->sh_type != SHT_PROGBITS)
        {
            continue;
        }

        const char *name = strtab + section->sh_name;
        uint64_t addr = section->sh_addr;
        uint64_t addr_real = addr & 0x7FFFFFFF; // Mask the address to fit within 31 bits
        uint64_t size = section->sh_size;

        // Print details about the section being loaded
        std::cout << "Loading ELF section '" << name << "' @"
                  << std::hex << addr << " (@" << addr_real << ") size="
                  << std::dec << size << std::endl;

        // Check if the section's end exceeds the provided data buffer's length
        if (addr_real + size > data_len)
        {
            std::cerr << "ELF section too big or offset too great" << std::endl;
            munmap(elf_data, st.st_size);
            close(fd);
            return 5;
        }

        // Copy section data to the appropriate location in the data buffer
        uint8_t *section_data = static_cast<uint8_t *>(elf_data) + section->sh_offset;
        for (uint64_t j = 0; j < size; j++)
        {
            data[addr_real + j] = section_data[j];
        }
    }

    // Clean up
    munmap(elf_data, st.st_size);
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
}
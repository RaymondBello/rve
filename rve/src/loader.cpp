
#include "loader.h"


int loadElf(const char *path, uint64_t path_len, uint8_t *data, uint64_t data_len)
{

    // Convert C string to std::string
    std::string filepath(path, path_len);
    printf("INFO: Loading ELF binary %s\n", filepath.c_str());

    // Open in binary mode
    std::ifstream elf_file(filepath, std::ios::binary);
    if (!elf_file.is_open())
    {
        printf("ERRO: Failed to open ELF file\n");
        return 1;
    }

    // Read the ELF header
    Elf64_Ehdr elf_header;
    elf_file.read(reinterpret_cast<char *>(&elf_header), sizeof(elf_header));
    if (elf_file.gcount() != sizeof(elf_header))
    {
        printf("ERRO: Failed to read ELF header\n");
        return 2;
    }
    printf("INFO: Read %0d bytes of ELF header\n", elf_file.gcount());

    printf("INFO: ELF header.e_type is %0d\n", elf_header.e_type);
    printf("INFO: ELF header.e_machine is %0d\n", elf_header.e_machine);
    printf("INFO: ELF header.e_version is %0d\n", elf_header.e_version);
    printf("INFO: ELF header.e_entry is %0d\n", elf_header.e_entry);
    printf("INFO: ELF header.e_phoff is %0d\n", elf_header.e_phoff);
    printf("INFO: ELF header.e_shoff is %0d\n", elf_header.e_shoff);
    printf("INFO: ELF header.e_flags is %0d\n", elf_header.e_flags);
    printf("INFO: ELF header.e_ehsize is %0d\n", elf_header.e_ehsize);
    printf("INFO: ELF header.e_phentsize is %0d\n", elf_header.e_phentsize);
    printf("INFO: ELF header.e_phnum is %0d\n", elf_header.e_phnum);
    printf("INFO: ELF header.e_shentsize is %0d\n", elf_header.e_shentsize);
    printf("INFO: ELF header.e_shnum is %0d\n", elf_header.e_shnum);
    printf("INFO: ELF header.e_shstrndx is %0d\n", elf_header.e_shstrndx);



    // Seek to section header table
    elf_file.seekg(elf_header.e_shoff, std::ios::beg);
    printf("INFO: Found %0d sections in ELF \n", elf_header.e_shnum);

    std::vector<Elf64_Shdr> sections(elf_header.e_shnum);
    elf_file.read(reinterpret_cast<char *>(sections.data()), elf_header.e_shnum * sizeof(Elf64_Shdr));
    if (elf_file.gcount() != static_cast<std::streamsize>(elf_header.e_shnum * sizeof(Elf64_Shdr)))
    {
        printf("ERRO: Failed to read ELF section header\n");
        return 2;
    }
    printf("INFO: Read %0d bytes of ELF section header\n", elf_file.gcount());

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
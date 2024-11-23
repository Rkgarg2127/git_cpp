#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>
#include "zstr.hpp"
#include <iomanip>
#include <openssl/sha.h>
using namespace std;

string compressedString(string data);
bool hashObject(string file);
std::string sha_file(std::string data);
std::string generateSHA1(const std::string &input) ;

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    // std::cout << "Logs from your program will appear here!\n";
    // std::cout << "argc: " << argc << '\n';

    // Uncomment this block to pass the first stage

    if (argc < 2)
    {
        std::cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }

    std::string command = argv[1];

    if (command == "init")
    {
        try
        {
            std::filesystem::create_directory(".git");
            std::filesystem::create_directory(".git/objects");
            std::filesystem::create_directory(".git/refs");

            std::ofstream headFile(".git/HEAD");
            if (headFile.is_open())
            {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            }
            else
            {
                std::cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }

            std::cout << "Initialized git directory\n";
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }
    else if (command == "cat-file")
    {
        if (argc < 4)
        {
            std::cerr << "Invalid command, required '-p <blob saha>'\n";
            return EXIT_FAILURE;
        }
        const std::string flag = argv[2];
        if (flag != "-p")
        {
            std::cerr << "Invalid or Absent flag, required '-p <blob sha>'\n";
            return EXIT_FAILURE;
        }
        const std::string value = argv[3];
        const std::string dir_name = value.substr(0, 2);
        const std::string blob_sha = value.substr(2);
        const std::string file_address = ".git/objects/" + dir_name + "/" + blob_sha;

        zstr::ifstream blob_input(file_address, std::ofstream::binary);
        if (!blob_input.is_open())
        {
            std::cerr << "Failed to open file\n";
            return EXIT_FAILURE;
        }
        std::string blob_content{std::istreambuf_iterator<char>(blob_input),
                                 std::istreambuf_iterator<char>()};
        blob_input.close();
        std::cout << blob_content.substr(blob_content.find('\0') + 1);
    }
    else if (command == "hash-object")
    {
        if (argc < 4)
        {
            std::cerr << "Invalid command, required '-w <file address>'\n";
            return EXIT_FAILURE;
        }
        const std::string flag = argv[2];
        if (flag != "-w")
        {
            std::cerr << "Invalid or Absent flag, required '-w <file address>'\n";
            return EXIT_FAILURE;
        }
        const std::string value = argv[3];
        bool result = hashObject(value);
        if (!result)
        {
            std::cerr << "Failed to hash object\n";
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool hashObject(string file)
{
    ifstream t(file, ios::in | ios::out | ios::app);
    if (!t.is_open())
    {
        cout << "Problem in opening " << file << " during make hash object.\n";
        return false;
    }
    stringstream data;
    data << t.rdbuf();

    string blobContent = "blob " + to_string(data.str().size()) + "\0" + data.str();
    string sha_hash = generateSHA1(blobContent);

    // compressing content
    string compressedData = compressedString(blobContent);
    if (compressedData == "")
    {
        return false;
    }

    string dir = ".git/objects/" + sha_hash.substr(0, 2);
    if (!filesystem::exists(dir))
    {
        if (filesystem::create_directory(dir))
        {
            cout << "Directory created\n";
        }
        else
        {
            cout << "Failed to create directory\n";
            return false;
        }
    }

    string file_address = dir + "/" +sha_hash.substr(2);
    std::ofstream objectFile(file_address, std::ios::binary);
    objectFile.write(compressedData.c_str(), compressedData.size());
    objectFile.close();
    return true;
}

std::string generateSHA1(const std::string &input) {
    unsigned char hash[SHA_DIGEST_LENGTH]; // SHA1 hash is 20 bytes
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    // Convert hash to hexadecimal string
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
}

std::string sha_file(std::string data)
{
    unsigned char hash[20];
    SHA1((unsigned char *)data.c_str(), data.size(), hash);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto &byte : hash)
    {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}

string compressedString(string data)
{
    uLong dataSize = data.size();
    uLong compressedSize = compressBound(dataSize);

    string compressedData(compressedSize, 0);
    int result = compress((Bytef *)compressedData.data(), &compressedSize, (const Bytef *)data.data(), dataSize);

    if (result != Z_OK)
    {
        std::cerr << "Compression failed\n";
        return "";
    }
    return compressedData;
}



// git add .
// git commit --allow-empty -m "[any message]"
// git push origin master
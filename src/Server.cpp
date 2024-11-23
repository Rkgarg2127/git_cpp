// #include <iostream>
// #include <filesystem>
// #include <fstream>
// #include <string>
// #include <zlib.h>
// #include "zstr.hpp"
// #include <iomanip>
// #include <openssl/sha.h>
// using namespace std;

// string compressedString(string data);
// bool hashObject(string file);
// std::string sha_file(std::string data);

// int main(int argc, char *argv[])
// {
//     // Flush after every std::cout / std::cerr
//     std::cout << std::unitbuf;
//     std::cerr << std::unitbuf;

//     // You can use print statements as follows for debugging, they'll be visible when running tests.
//     // std::cout << "Logs from your program will appear here!\n";
//     // std::cout << "argc: " << argc << '\n';

//     // Uncomment this block to pass the first stage

//     if (argc < 2)
//     {
//         std::cerr << "No command provided.\n";
//         return EXIT_FAILURE;
//     }

//     std::string command = argv[1];

//     if (command == "init")
//     {
//         try
//         {
//             std::filesystem::create_directory(".git");
//             std::filesystem::create_directory(".git/objects");
//             std::filesystem::create_directory(".git/refs");

//             std::ofstream headFile(".git/HEAD");
//             if (headFile.is_open())
//             {
//                 headFile << "ref: refs/heads/main\n";
//                 headFile.close();
//             }
//             else
//             {
//                 std::cerr << "Failed to create .git/HEAD file.\n";
//                 return EXIT_FAILURE;
//             }

//             std::cout << "Initialized git directory\n";
//         }
//         catch (const std::filesystem::filesystem_error &e)
//         {
//             std::cerr << e.what() << '\n';
//             return EXIT_FAILURE;
//         }
//     }
//     else if (command == "cat-file")
//     {
//         if (argc < 4)
//         {
//             std::cerr << "Invalid command, required '-p <blob saha>'\n";
//             return EXIT_FAILURE;
//         }
//         const std::string flag = argv[2];
//         if (flag != "-p")
//         {
//             std::cerr << "Invalid or Absent flag, required '-p <blob sha>'\n";
//             return EXIT_FAILURE;
//         }
//         const std::string value = argv[3];
//         const std::string dir_name = value.substr(0, 2);
//         const std::string blob_sha = value.substr(2);
//         const std::string file_address = ".git/objects/" + dir_name + "/" + blob_sha;

//         zstr::ifstream blob_input(file_address, std::ofstream::binary);
//         if (!blob_input.is_open())
//         {
//             std::cerr << "Failed to open file\n";
//             return EXIT_FAILURE;
//         }
//         std::string blob_content{std::istreambuf_iterator<char>(blob_input),
//                                  std::istreambuf_iterator<char>()};
//         blob_input.close();
//         std::cout << blob_content.substr(blob_content.find('\0') + 1);
//     }
//     else if (command == "hash-object")
//     {
//         if (argc < 4)
//         {
//             std::cerr << "Invalid command, required '-w <file address>'\n";
//             return EXIT_FAILURE;
//         }
//         const std::string flag = argv[2];
//         if (flag != "-w")
//         {
//             std::cerr << "Invalid or Absent flag, required '-w <file address>'\n";
//             return EXIT_FAILURE;
//         }
//         const std::string value = argv[3];
//         bool result = hashObject(value);
//         if (!result)
//         {
//             std::cerr << "Failed to hash object\n";
//             return EXIT_FAILURE;
//         }
//     }
//     else
//     {
//         std::cerr << "Unknown command " << command << '\n';
//         return EXIT_FAILURE;
//     }

//     return EXIT_SUCCESS;
// }

// bool hashObject(string file)
// {
//     ifstream t(file, ios::in | ios::out | ios::app);
//     if (!t.is_open())
//     {
//         cout << "Problem in opening " << file << " during make hash object.\n";
//         return false;
//     }
//     stringstream data;
//     data << t.rdbuf();

//     string blobContent = "blob " + to_string(data.str().size()) + "\0" + data.str();
//     string sha_hash = sha_file(blobContent);

//     // compressing content
//     string compressedData = compressedString(blobContent);
//     if (compressedData == "")
//     {
//         return false;
//     }

//     string dir = ".git/objects/" + sha_hash.substr(0, 2);
//     if (!filesystem::exists(dir))
//     {
//         if (filesystem::create_directory(dir))
//         {
//             cout << "Directory created\n";
//         }
//         else
//         {
//             cout << "Failed to create directory\n";
//             return false;
//         }
//     }

//     string file_address = dir + "/" +sha_hash.substr(2);
//     std::ofstream objectFile(file_address, std::ios::binary);
//     objectFile.write(compressedData.c_str(), compressedData.size());
//     objectFile.close();
//     return true;
// }

// std::string sha_file(std::string data)
// {
//     unsigned char hash[20];
//     SHA1((unsigned char *)data.c_str(), data.size(), hash);
//     std::stringstream ss;
//     ss << std::hex << std::setfill('0');
//     for (const auto &byte : hash)
//     {
//         ss << std::setw(2) << static_cast<int>(byte);
//     }
//     std::cout << ss.str() << std::endl;
//     return ss.str();
// }

// string compressedString(string data)
// {
//     uLong dataSize = data.size();
//     uLong compressedSize = compressBound(dataSize);

//     string compressedData(compressedSize, 0);
//     int result = compress((Bytef *)compressedData.data(), &compressedSize, (const Bytef *)data.data(), dataSize);

//     if (result != Z_OK)
//     {
//         std::cerr << "Compression failed\n";
//         return "";
//     }
//     return compressedData;
// }

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>
#include <vector>
#include "Error.h"
#include <openssl/sha.h>
Error  decompressFile(const std::string& file_path);
std::string sha_file(std::string basicString);
void compressFile(std::string basicString, uLong *pInt, unsigned char data[20]);
void compressFile(const std::string data, uLong *bound, unsigned char *dest) ;
void hash_object(std::string file);
int main(int argc, char *argv[])
{
/*
    std::cout << "Logs from your program will appear here!\n";
*/
    if (argc < 2) {
        std::cerr << "No command provided.\n";
        return EXIT_FAILURE;
    }
    std::string command = argv[1];
    if (command == "init") {
        try {
            std::filesystem::create_directory(".git");
            std::filesystem::create_directory(".git/objects");
            std::filesystem::create_directory(".git/refs");
            std::ofstream headFile(".git/HEAD");
            if (headFile.is_open()) {
                headFile << "ref: refs/heads/main\n";
                headFile.close();
            } else {
                std::cerr << "Failed to create .git/HEAD file.\n";
                return EXIT_FAILURE;
            }
            std::cout << "Initialized git directory\n";
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }
    else if (command=="cat-file"){
        std::string  blob=argv[2];
        if (blob=="-p"){
            blob=argv[3];
        }
        std::string file_path = ".git/objects/" + blob.substr(0, 2) +"/" + blob.substr(2);
        Error err=decompressFile(file_path);
        if (!err.Error){
            return  EXIT_FAILURE;
        }
    }
    else if (command=="hash-object"){
        if(argc<3){
            std::cerr << "Less than 3 argument "<< '\n';
            return EXIT_FAILURE;
        }
        std::string hash=argv[3];
        hash_object(hash);
    }
    else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
Error  decompressFile(const std::string& file_path){
    Error error;
    std::ifstream file(file_path,std::ios::binary);
    if (!file.is_open()){
        std::cerr<<"Cant Open File"<<file_path<<std::endl;
        error.SetMessage("Cant Open File");
        error.SetBool(false);
        return error;
    }
    //std::vector<char> compressedData((std::istreambuf_iterator<char>)(file),std::istreambuf_iterator<char>());
    std::vector<char> compressedData;
    char c;
    while (file.get(c)){
        compressedData.push_back(c);
    }
    file.close();
    std::vector<char> deCompressedData;
    deCompressedData.resize(compressedData.size()*4);
    z_stream strm;
    strm.zalloc=Z_NULL;
    strm.zfree=Z_NULL;
    strm.opaque=Z_NULL;
    strm.avail_in=compressedData.size();
    strm.next_in=reinterpret_cast<Bytef*>(compressedData.data());
    strm.avail_out =deCompressedData.size();
    strm.next_out = reinterpret_cast<Bytef*>(deCompressedData.data());
    int ret = inflateInit(&strm);
    if (ret != Z_OK) {
        std::cerr << "error,init failed" << std::endl;
        inflateEnd(&strm);
        error.SetMessage("error,inti failed");
        error.SetBool(false);
        return error;
    }
    ret= inflate(&strm,Z_FINISH);
    if (ret!=Z_STREAM_END){
        std::cerr << "error,inflate failed" << std::endl;
        inflateEnd(&strm);
        error.SetBool(false);
        error.SetMessage("error,inflate");
        return error;
    }
    inflateEnd(&strm);
    deCompressedData.resize(strm.total_out);
    bool delimeter = false;
    for (char i : deCompressedData) {
        if (delimeter) {
            std::cout << i;
        }
        if (i == '\0')
            delimeter = true;
    }
    error.SetBool(true);
    return  error;
}
void hash_object(std::string file) {
    // Read file contents
    std::ifstream t(file);
    std::stringstream data;
    data << t.rdbuf();
    // Create blob content string
    std::string content = "blob " + std::to_string(data.str().length()) + '\0' + data.str();
    // Calculate SHA1 hash
    std::string buffer = sha_file(content);
    // Compress blob content
    uLong bound = compressBound(content.size());
    unsigned char compressedData[bound];
    compressFile(content, &bound, compressedData);
    // Write compressed data to .git/objects
    std::string dir = ".git/objects/" + buffer.substr(0,2);
    std::filesystem::create_directory(dir);
    std::string objectPath = dir + "/" + buffer.substr(2);
    std::ofstream objectFile(objectPath, std::ios::binary);
    objectFile.write((char *)compressedData, bound);
    objectFile.close();
}
void compressFile(const std::string data, uLong *bound, unsigned char *dest) {
    compress(dest, bound, (const Bytef *)data.c_str(), data.size());
}
std::string sha_file(std::string data) {
    unsigned char hash[20];
    SHA1((unsigned char *)data.c_str(), data.size(), hash);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : hash) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>
#include "zstr.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    // std::cout << "Logs from your program will appear here!\n";
    // std::cout << "argc: " << argc << '\n';

    // Uncomment this block to pass the first stage
    
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
    else if(command=="cat-file"){
        if (argc<4)
        {
            std::cerr<<"Invalid command, required '-p <blob saha>'\n";
            return EXIT_FAILURE;
        }
        const std::string flag=argv[2];
        if(flag!="-p"){
            std::cerr<<"Invalid or Absent flag, required '-p <blob sha>'\n";
            return EXIT_FAILURE;
        }
        const std::string value=argv[3];
        const std::string dir_name= value.substr(0,2);
        const std::string blob_sha= value.substr(2);
        const std::string file_address=".git/objects/"+dir_name+"/"+blob_sha;

        zstr:: ifstream blob_input(file_address , std::ofstream::binary);
        if(!blob_input.is_open()){
            std::cerr<<"Failed to open file\n";
            return EXIT_FAILURE;
        }
        std::string blob_content{std::istreambuf_iterator<char>(blob_input),
                         std::istreambuf_iterator<char>()};
        blob_input.close();
        std::cout<<blob_content.substr(blob_content.find('\0') + 1);
    }
    else {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <zlib.h>
#include "zstr.hpp"
#include <chrono>
#include <ctime>
#include <algorithm>
#include <string>
#include <openssl/sha.h>
using namespace std;

string makeBlob(string file);
string makeTree(string directoryAddress);
string makeCompressedObject(string input);
string compressedString(string data);
string generateSHA1(const std::string &input);
string fromHex(const std::string &hexStr);
string getFormattedTimestamp();

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
        string result = makeBlob(value);
        if (result == "")
        {
            std::cerr << "Failed to hash object\n";
            return EXIT_FAILURE;
        }
        cout << result;
    }
    else if (command == "ls-tree")
    {
        if (argc < 4)
        {
            std::cerr << "Invalid command, required ' --name-only <tree_sha> '\n";
            return EXIT_FAILURE;
        }
        const std::string value = argv[3];
        const std::string dir_name = value.substr(0, 2);
        const std::string tree_sha = value.substr(2);
        const std::string file_address = ".git/objects/" + dir_name + "/" + tree_sha;

        zstr::ifstream tree_input(file_address, std::ofstream::binary);
        if (!tree_input.is_open())
        {
            std::cerr << "Failed to open file\n";
            return EXIT_FAILURE;
        }
        std::string tree_content{std::istreambuf_iterator<char>(tree_input),
                                 std::istreambuf_iterator<char>()};
        tree_input.close();

        // string Stream example:- "tree 96\x0040000 doo\x00pPC!\xb4\xde>\xf8\x88ܷ\xb6H\x17z,6.\x01\xba40000 dooby\x00赂\xbdd\xd8\xc1)E\x12\xe3H֟d\xb0=@q'100644 humpty\x00ؘ\x9d\x89\xa4/\xcc\xc5r8\x1e\xfb\x9d\x94a\xfe\xf1\x94\xf9~"
        // required:- "doo\ndooby\nhumpty\n"

        // Spliting the stream into entries on basis of " "
        vector<string> tree_enterires;
        std::string line;
        stringstream ss(tree_content);
        while (getline(ss, line, ' '))
        {
            tree_enterires.push_back(line);
        }
        string res;
        for (int i = 2; i < tree_enterires.size(); i++)
        {
            string entry = tree_enterires[i];
            int pos = entry.find('\0');
            res += entry.substr(0, pos) + '\n';
        }
        cout << res;

        // std::cout << tree_content.substr(tree_content.find('\0') + 1);
        return EXIT_SUCCESS;
    }
    else if (command == "write-tree")
    {
        if (argc < 2)
        {
            std::cerr << "Invalid command, required 'write-tree'\n";
            return EXIT_FAILURE;
        }

        string generartedTreeHash = makeTree(".");
        if (generartedTreeHash == "")
        {
            cout << "Some ERROR";
            return EXIT_FAILURE;
        }
        else
        {
            cout << generartedTreeHash;
        }
    }
    else if (command == "commit-tree")
    {
        // creating commmit object
        if (argc < 7)
        {
            // Invalid input Required :  ./your_program.sh commit-tree <tree_sha> -p <commit_sha> -m <message>
            cout << "INvalid command.\n REquired:  ./your_program.sh commit-tree <tree_sha> -p <commit_sha> -m <message>\n";
        }

        // Taking hardcoded user data
        string author_name = "Ram Kinkar", author_email = "rkgarg15042005@gmail.com";
        string commiter_name = "Ram Kinkar", commiter_email = "rkgarg15042005@gmail.com";
        string timestamp = getFormattedTimestamp();

        string tree_sha = argv[2];
        string parent_sha = argv[4];
        string message = argv[6];

        // Computing commit Content

        string commitData = "tree " + tree_sha + '\n';
        if (!parent_sha.empty())
        {
            commitData += "parent " + parent_sha + '\n';
        }
        commitData += "author " + author_name + " <" + author_email + "> " + timestamp + "\n";
        commitData += "committer " + commiter_name + " <" + commiter_email + "> " + timestamp + "\n";
        commitData += "\n" + message+"\n";

        // Example of Commit Data:-
        // // tree d8329fc1cc938780ffdd9f94e0d364e0ea74f579
        // // author Scott Chacon <schacon@gmail.com> 1243040974 -0700
        // // committer Scott Chacon <schacon@gmail.com> 1243040974 -0700
        // //
        // // First commit

        // cout<<commitData<<endl<<endl;

        commitData = "commit " + to_string(commitData.size()) + '\0' + commitData;
        string commitSha = makeCompressedObject(commitData);
        if (commitSha == "")
        {
            cout << "Some ERROR";
            return EXIT_FAILURE;
        }
        else
        {
            cout << commitSha;
        }
    }
    else
    {
        std::cerr << "Unknown command " << command << '\n';
        return EXIT_FAILURE;
    }
}

string makeBlob(string file)
{
    ifstream t(file, ios::in | ios::out | ios::app);
    if (!t.is_open())
    {
        cout << "Problem in opening " << file << " during make hash object.\n";
        return "";
    }
    stringstream data;
    data << t.rdbuf();

    string blobContent = "blob " + to_string(data.str().size()) + '\0' + data.str();

    string sha_hash = makeCompressedObject(blobContent);
    return sha_hash;
}

string makeTree(string directoryAddress)
{
    if (!filesystem::exists(directoryAddress) || !filesystem::is_directory(directoryAddress))
    {
        std::cerr << "Directory:" + directoryAddress + " does not exist or is not accessible!" << std::endl;
        return "";
    }

    vector<pair<string, pair<string, string>>> files;
    string treeContent = "";
    int fileCount = 0;
    for (const auto &entry : filesystem::directory_iterator(directoryAddress))
    {
        fileCount++;
        string fileName = entry.path().filename().string();
        string subHash;
        string fileMode;
        if (fileName == ".git" || fileName.front() == '.')
        {
            continue;
        }
        else if (filesystem::is_directory(entry.path()))
        {
            subHash = makeTree(entry.path().string());
            fileMode = "40000";
        }
        else
        {
            subHash = makeBlob(entry.path().string());
            fileMode = "100644";
        }
        if (subHash == "")
        {
            cout << "Some ERROR";
            return "";
        }
        string hexSubHash = fromHex(subHash);
        files.push_back({fileName, {fileMode, hexSubHash}});
    }

    sort(files.begin(), files.end());
    for (int i = 0; i < files.size(); i++)
    {
        // printf("%s %s %s\n", files[i].second.first.c_str(), files[i].first.c_str(), files[i].second.second.c_str());
        treeContent += files[i].second.first + " " + files[i].first + '\0' + files[i].second.second;
    }

    treeContent = "tree " + to_string(treeContent.size()) + '\0' + treeContent;
    // cout<<treeContent<<endl<<endl;
    string sha_hash = makeCompressedObject(treeContent);
    return sha_hash;
}

string makeCompressedObject(string input)
{

    string sha_hash = generateSHA1(input);

    // compressing content
    string compressedData = compressedString(input);
    if (compressedData == "")
    {
        cout << "empty compressedData";
        return "";
    }

    string dir = ".git/objects/" + sha_hash.substr(0, 2);
    if (!filesystem::exists(dir))
    {
        if (filesystem::create_directory(dir))
        {
            // cout << "Directory created\n";
        }
        else
        {
            cout << "Failed to create directory\n";
            return "";
        }
    }

    string file_address = dir + "/" + sha_hash.substr(2);
    ofstream objectFile(file_address, ios::binary);
    objectFile.write(compressedData.c_str(), compressedData.size());
    objectFile.close();
    return sha_hash;
}

string generateSHA1(const std::string &input)
{
    unsigned char hash[SHA_DIGEST_LENGTH]; // SHA1 hash is 20 bytes
    SHA1(reinterpret_cast<const unsigned char *>(input.c_str()), input.size(), hash);

    // Convert hash to hexadecimal string
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
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
    compressedData.resize(compressedSize);
    return compressedData;
}

string fromHex(const std::string &hexStr)
{
    std::string result;
    for (size_t i = 0; i < hexStr.length(); i += 2)
    {
        std::string byteStr = hexStr.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
        result += byte;
    }
    return result;
}

string getFormattedTimestamp()
{
    // Get the current time
    auto now = std::chrono::system_clock::now();
    std::time_t unix_timestamp = std::chrono::system_clock::to_time_t(now);

    // Get the local timezone offset in seconds
    std::tm local_tm = *std::localtime(&unix_timestamp);
    std::tm utc_tm = *std::gmtime(&unix_timestamp);
    int offset_seconds = std::difftime(std::mktime(&local_tm), std::mktime(&utc_tm));

    // Calculate the offset in hours and minutes
    int offset_hours = offset_seconds / 3600;
    int offset_minutes = std::abs(offset_seconds % 3600 / 60);

    // Format the timezone offset
    std::ostringstream offset_stream;
    offset_stream << (offset_hours >= 0 ? "+" : "-")                             // Sign
                  << std::setw(2) << std::setfill('0') << std::abs(offset_hours) // Hours
                  << std::setw(2) << std::setfill('0') << offset_minutes;        // Minutes

    // Combine Unix timestamp and timezone offset
    std::ostringstream formatted_timestamp;
    formatted_timestamp << unix_timestamp << " " << offset_stream.str();

    return formatted_timestamp.str();
}

// git add .
// git commit --allow-empty -m "[any message]"
// git push origin master
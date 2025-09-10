#include <vector>
#include <filesystem>

class DirectoryManager {
private:
    std::vector<std::filesystem::path> directories;
public:
    DirectoryManager();
    ~DirectoryManager();
    std::vector<std::filesystem::path> findFiles(const std::filesystem::path& path, const std::string& extension);
};
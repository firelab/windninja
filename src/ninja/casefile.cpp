#include "casefile.h"

class CaseFile {
public:
    void addFileToZip(const std::string& zipFilePath, const std::string& fileToAdd, const std::string& zipEntryName) {
        VSILFILE *fin;
        fin = VSIFOpenL(fileToAdd.c_str(), "r");
        if (fin == NULL) {
            std::cerr << "Failed to open file: " << fileToAdd << std::endl;
            return;
        }

        vsi_l_offset offset;
        VSIFSeekL(fin, 0, SEEK_END);
        offset = VSIFTellL(fin);
        VSIRewindL(fin);

        char *data = (char*)CPLMalloc(offset * sizeof(char));
        if (data == NULL) {
            std::cerr << "Failed to allocate memory for file data." << std::endl;
            VSIFCloseL(fin);
            return;
        }
        if (VSIFReadL(data, 1, offset, fin) != offset) {
            std::cerr << "Failed to read file contents: " << fileToAdd << std::endl;
            CPLFree(data);
            VSIFCloseL(fin);
            return;
        }
        VSIFCloseL(fin);

        void* zipHandle = CPLCreateZip(zipFilePath.c_str(), NULL);
        if (zipHandle == NULL) {
            std::cerr << "Failed to create or open zip file: " << zipFilePath << std::endl;
            CPLFree(data);
            return;
        }

        if (CPLCreateFileInZip(zipHandle, zipEntryName.c_str(), NULL) != CE_None) {
            std::cerr << "Failed to create file in zip: " << zipEntryName << std::endl;
            CPLFree(data);
            CPLCloseZip(zipHandle);
            return;
        }

        if (CPLWriteFileInZip(zipHandle, data, static_cast<int>(offset)) != CE_None) {
            std::cerr << "Failed to write data to file in zip: " << zipEntryName << std::endl;
        }

        if (CPLCloseFileInZip(zipHandle) != CE_None) {
            std::cerr << "Failed to close file in zip: " << zipEntryName << std::endl;
        }

        CPLCloseZip(zipHandle);

        CPLFree(data);

        std::cout << "File added to ZIP: " << zipEntryName << std::endl;
    }

    void deleteFileFromPath(std::string directoryPath, std::string Filename) {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().filename() == Filename) {
                std::filesystem::remove(entry.path()); // Use std::filesystem explicitly
                std::cout << "Deleted file: " << entry.path() << std::endl;
            }
        }
    }
};

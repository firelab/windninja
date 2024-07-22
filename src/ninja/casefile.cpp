#include "casefile.h"

std::string CaseFile::zipfilename = "";
std::string CaseFile::directory = "";

std::mutex zipMutex;

CaseFile::CaseFile() {
}

bool CaseFile::isCfgFile(const std::string& filePath) {
    const std::string extension = ".cfg";
    if (filePath.length() >= extension.length()) {
        std::string fileExtension = filePath.substr(filePath.length() - extension.length());
        return fileExtension == extension;
    } else {
        return false;
    }
}

bool CaseFile::isVTKFile(const std::string& filePath) {
    const std::string extension = ".vtk";
    if (filePath.length() >= extension.length()) {
        std::string fileExtension = filePath.substr(filePath.length() - extension.length());
        return fileExtension == extension;
    } else {
        return false;
    }
}

bool CaseFile::lookforzip(const std::string& zipFilePath, const std::string& directory) {
    char** papszDir = VSIReadDir(directory.c_str());
    if (papszDir != nullptr) {
        
    for (int i = 0; papszDir[i] != nullptr; i++) {
        std::string entry = papszDir[i];

        if (entry == "." || entry == "..") {
            continue;
        }

        if (entry == parse("file", getzip())) {
            return true;
        }
    }

    CSLDestroy(papszDir);
    }
    return false;
}

bool CaseFile::lookfordate(const std::string& date) {
    return false; 
}


std::string CaseFile::parse(const std::string& type, const std::string& path) {
            size_t found = path.find_last_of("/");
            if (found != std::string::npos) {
                if (strcmp(type.c_str(), "directory") == 0) {
                    return path.substr(0, found); 

                }
                else 
                if (strcmp(type.c_str(), "file") == 0) {
                    return path.substr(found + 1); // Extract substring after the last '/'
                }
            }
}


void CaseFile::addFileToZip(const std::string& zipFilePath, const std::string& dirPath, const std::string& fileToAdd, const std::string& usrlocalpath) {
    // Enable CPL logging
    std::lock_guard<std::mutex> lock(zipMutex); // for multithreading issue?? not sure I think i busted it
    CPLSetConfigOption("CPL_DEBUG", "ON");

    std::cout << "ZIP File Path: " << zipFilePath << std::endl;
    std::cout << "Directory Path: " << dirPath << std::endl;
    std::cout << "File to Add Path: " << usrlocalpath << std::endl;
    std::cout << "File to Add: " << fileToAdd << std::endl;

    bool foundzip = lookforzip(zipFilePath, dirPath);
    std::cout << "Found ZIP: " << foundzip << std::endl;

    if (foundzip) {
        std::ifstream infile(zipFilePath);
        if (!infile.good()) {
            std::cerr << "ZIP file does not exist: " << zipFilePath << std::endl;
            return;
        }
    }

    zipFile zip;
    if (!foundzip) {
        zip = cpl_zipOpen(zipFilePath.c_str(), APPEND_STATUS_CREATE);
        std::cout << "Creating new ZIP file: " << zipFilePath << std::endl;
    } else {
        zip = cpl_zipOpen(zipFilePath.c_str(), APPEND_STATUS_ADDINZIP);
        std::cout << "Appending to existing ZIP file: " << zipFilePath << std::endl;
    }

    if (zip == NULL) {
        std::cerr << "Failed to open ZIP file: " << zipFilePath << std::endl;
        CPLError(CE_Failure, CPLE_OpenFailed, "Failed to open ZIP file %s", zipFilePath.c_str());
        const char* errMsg = CPLGetLastErrorMsg();
        std::cerr << "GDAL Error: " << errMsg << std::endl;
        return;
    }

    zip_fileinfo zi = {0};
    if (cpl_zipOpenNewFileInZip(zip, fileToAdd.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        std::cerr << "Could not open new file in ZIP: " << fileToAdd << std::endl;
        CPLError(CE_Failure, CPLE_FileIO, "Could not open new file in ZIP %s", fileToAdd.c_str());
        cpl_zipClose(zip, nullptr);
        return;
    }

    VSILFILE *file = VSIFOpenL(usrlocalpath.c_str(), "rb");
    if (file == nullptr) {
        std::cerr << "Could not open file for reading with VSIL: " << usrlocalpath << std::endl;
        CPLError(CE_Failure, CPLE_FileIO, "Could not open file for reading with VSIL %s", usrlocalpath.c_str());
        cpl_zipCloseFileInZip(zip);
        cpl_zipClose(zip, nullptr);
        return;
    }

    VSIFSeekL(file, 0, SEEK_END);
    vsi_l_offset fileSize = VSIFTellL(file);
    VSIFSeekL(file, 0, SEEK_SET);

    char *data = (char*)CPLMalloc(fileSize);
    if (data == nullptr) {
        std::cerr << "Failed to allocate memory for file data." << std::endl;
        CPLError(CE_Failure, CPLE_OutOfMemory, "Failed to allocate memory for file data.");
        VSIFCloseL(file);
        cpl_zipCloseFileInZip(zip);
        cpl_zipClose(zip, nullptr);
        return;
    }

    if (VSIFReadL(data, 1, fileSize, file) != fileSize) {
        std::cerr << "Failed to read file contents: " << fileToAdd << std::endl;
        CPLError(CE_Failure, CPLE_FileIO, "Failed to read file contents %s", fileToAdd.c_str());
        CPLFree(data);
        VSIFCloseL(file);
        cpl_zipCloseFileInZip(zip);
        cpl_zipClose(zip, nullptr);
        return;
    }

    if (cpl_zipWriteInFileInZip(zip, data, static_cast<unsigned int>(fileSize)) != ZIP_OK) {
        std::cerr << "Error writing data to ZIP file: " << fileToAdd << std::endl;
        CPLError(CE_Failure, CPLE_FileIO, "Error writing data to ZIP file %s", fileToAdd.c_str());
    }

    CPLFree(data);
    VSIFCloseL(file);
    cpl_zipCloseFileInZip(zip);

    if (cpl_zipClose(zip, nullptr) != ZIP_OK) {
        std::cerr << "Error closing ZIP file" << std::endl;
        CPLError(CE_Failure, CPLE_FileIO, "Error closing ZIP file %s", zipFilePath.c_str());
    }
    std::cout << "File added to ZIP: " << fileToAdd << std::endl; 
}



std::string CaseFile::getTime() {
            auto now = std::chrono::system_clock::now(); 
    
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    
            std::tm* local_tm = std::localtime(&now_time_t);
    
            std::ostringstream oss;
            oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S"); 
            return oss.str();
}


void CaseFile::deleteFileFromPath(std::string directoryPath, std::string filename) {
     
    char** papszDir = VSIReadDir(directoryPath.c_str());
    if (papszDir != nullptr) {
        
    for (int i = 0; papszDir[i] != nullptr; i++) {
        std::string entry = papszDir[i];

        if (entry == "." || entry == "..") {
            continue;
        }

        std::string fullPath = directoryPath + "/" + entry;

        if (entry == filename) {
        VSIUnlink(fullPath.c_str()); 
        }

    }

    CSLDestroy(papszDir);
    }
}
   void CaseFile::setdir(std::string dir) {
        directory = dir; 
    }

   std::string CaseFile::getzip() {
        return zipfilename;
    }

   void CaseFile::setzip(std::string zip) {
        zipfilename = zip; 
    }

   std::string CaseFile::getdir() {
        return directory;
    }

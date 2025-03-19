#include "casefile.h"

//std::mutex zipMutex;

CaseFile::CaseFile()
{
    zipfilename = "";
    directory = "";
    zipalreadyopened = false;

    downloadedfromdem = false;
    elevsource = "";
}

bool CaseFile::isCfgFile(const std::string& filePath)
{
    const std::string extension = ".cfg";
    if (filePath.length() >= extension.length())
    {
        std::string fileExtension = filePath.substr(filePath.length() - extension.length());
        return fileExtension == extension;
    } else
    {
        return false;
    }
}

bool CaseFile::isVTKFile(const std::string& filePath)
{
    const std::string extension = ".vtk";
    if (filePath.length() >= extension.length())
    {
        std::string fileExtension = filePath.substr(filePath.length() - extension.length());
        return fileExtension == extension;
    } else
    {
        return false;
    }
}

bool CaseFile::lookforzip(const std::string& zipFilePath, const std::string& directory)
{
    char** papszDir = VSIReadDir(directory.c_str());
    if (papszDir != nullptr)
    {
        for (int i = 0; papszDir[i] != nullptr; i++)
        {
            std::string entry = papszDir[i];

            if (entry == "." || entry == "..")
            {
                continue;
            }

            if (entry == parse("file", getzip()))
            {
                return true;
            }
        }
        CSLDestroy(papszDir);
    }
    return false;
}

bool CaseFile::lookfordate(const std::string& date)
{
    return false;
}

std::string CaseFile::parse(const std::string& type, const std::string& path)
{
    size_t found = path.find_last_of("/");
    if (found != std::string::npos)
    {
        if (strcmp(type.c_str(), "directory") == 0)
        {
            return path.substr(0, found);
        } else
        {
            if (strcmp(type.c_str(), "file") == 0)
            {
                return path.substr(found + 1); // Extract substring after the last '/'
            }
        }
    } else
    {
        //std::cout << "couldn't parse" << std::endl;
        //return "";
        return path;
    }
}

void CaseFile::addFileToZip(const std::string& zipFilePath, const std::string& dirPath, const std::string& fileToAdd, const std::string& usrlocalpath)
{
    // CPL logging enabled here 
    //std::lock_guard<std::mutex> lock(zipMutex); // for multithreading issue
    //CPLSetConfigOption("CPL_DEBUG", "ON");

    try {
        bool foundzip = lookforzip(zipFilePath, dirPath);

        if (foundzip)
        {
            std::ifstream infile(zipFilePath);
            if (!infile.good())
            {
                CPLDebug("ZIP", "ZIP file does not exist: %s", zipFilePath.c_str());
                return;
            }
        }

        zipFile zip;
        if (!foundzip)
        {
            zip = cpl_zipOpen(zipFilePath.c_str(), APPEND_STATUS_CREATE);
        } else
        {
            zip = cpl_zipOpen(zipFilePath.c_str(), APPEND_STATUS_ADDINZIP);
        }

        if (zip == NULL)
        {
            CPLDebug("ZIP", "Could not open ZIP: %s", zipFilePath.c_str());
            return;
        }

        zip_fileinfo zi = {0};
        if (cpl_zipOpenNewFileInZip(zip, fileToAdd.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
        {
            CPLDebug("ZIP", "Could not open new file in ZIP: %s", fileToAdd.c_str());
            cpl_zipClose(zip, nullptr);
            return;
        }

        VSILFILE *file = VSIFOpenL(usrlocalpath.c_str(), "rb");
        if (file == nullptr)
        {
            CPLDebug("VSIL", "Could not open file for reading with VSIL: %s", usrlocalpath.c_str());
            cpl_zipCloseFileInZip(zip);
            cpl_zipClose(zip, nullptr);
            return;
        }

        VSIFSeekL(file, 0, SEEK_END);
        vsi_l_offset fileSize = VSIFTellL(file);
        VSIFSeekL(file, 0, SEEK_SET);

        char *data = (char*)CPLMalloc(fileSize);
        if (data == nullptr)
        {
            CPLDebug("Memory", "Failed to allocate memory for file data.");
            VSIFCloseL(file);
            cpl_zipCloseFileInZip(zip);
            cpl_zipClose(zip, nullptr);
            return;
        }

        if (VSIFReadL(data, 1, fileSize, file) != fileSize)
        {
            CPLDebug("FileRead", "Failed to read file contents: %s", fileToAdd.c_str());
            CPLFree(data);
            VSIFCloseL(file);
            cpl_zipCloseFileInZip(zip);
            cpl_zipClose(zip, nullptr);
            return;
        }

        if (cpl_zipWriteInFileInZip(zip, data, static_cast<unsigned int>(fileSize)) != ZIP_OK)
        {
            CPLDebug("ZIP", "Error writing data to ZIP file: %s", fileToAdd.c_str());
        }

        CPLFree(data);
        VSIFCloseL(file);
        cpl_zipCloseFileInZip(zip);

        if (cpl_zipClose(zip, nullptr) != ZIP_OK)
        {
            CPLDebug("ZIP", "Error closing ZIP file: %s", zipFilePath.c_str());
        }

    } catch (const std::exception& e)
    {
        CPLDebug("Exception", "Caught exception: %s", e.what());
        CPLError(CE_Failure, CPLE_AppDefined, "Exception caught: %s", e.what());
    } catch (...)
    {
        CPLDebug("Exception", "Caught unknown exception.");
        CPLError(CE_Failure, CPLE_AppDefined, "Caught unknown exception.");
    }
}

std::string CaseFile::getTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm* local_tm = std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// to avoid renaming the casefile except for the first run/ninja, checking for a specific starting zip file name
void CaseFile::rename(std::string newname)
{
    if (parse("file", zipfilename) == "tmp.ninja")
    {
        std::string oldFilePath = zipfilename;
        std::string newFilePath = newname;

        if (VSIRename(oldFilePath.c_str(), newFilePath.c_str()) == 0)
        {
            CPLDebug("ZIP_RENAME", "Successfully renamed %s to %s", oldFilePath.c_str(), newFilePath.c_str());
            zipfilename = newname;
        } else
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to rename %s to %s", oldFilePath.c_str(), newFilePath.c_str());
        }
    }
}

void CaseFile::deleteFileFromPath(std::string directoryPath, std::string filename)
{
    char** papszDir = VSIReadDir(directoryPath.c_str());
    if (papszDir != nullptr)
    {
        for (int i = 0; papszDir[i] != nullptr; i++)
        {
            std::string entry = papszDir[i];

            if (entry == "." || entry == "..")
            {
                continue;
            }

            std::string fullPath = directoryPath + "/" + entry;

            if (entry == filename)
            {
                VSIUnlink(fullPath.c_str());
            }
        }
        CSLDestroy(papszDir);
    }
}

void CaseFile::setdir(std::string dir)
{
    directory = dir;
}

std::string CaseFile::getzip()
{
    return zipfilename;
}

void CaseFile::setzip(std::string zip)
{
    zipfilename = zip;
}

std::string CaseFile::getdir()
{
    return directory;
}

void CaseFile::setZipOpen(bool zipopen)
{
    if (zipopen)
    {
        zipalreadyopened = true;
    } else
    {
        zipalreadyopened = false;
    }
}

bool CaseFile::getZipOpen()
{
    return zipalreadyopened;
}

void CaseFile::setTimeWX(std::vector<boost::local_time::local_date_time> timeList)
{
    timesforWX = timeList;
}

std::vector<boost::local_time::local_date_time> CaseFile::getWXTIME()
{
    return timesforWX;
}

void CaseFile::setBoundingBox(std::vector<double> boundingboxarrr)
{
    boundingboxarr = boundingboxarrr;
}

void CaseFile::setElevSource(std::string elevsourcee)
{
    elevsource = elevsourcee;
}

void CaseFile::setDownloadedFromDEM(bool downloadedfromdemm)
{
    downloadedfromdem = downloadedfromdemm;
}

std::string CaseFile::getElevSource()
{
    return elevsource;
}

bool CaseFile::getDownloadedFromDEM()
{
    return downloadedfromdem;
}

std::vector<double> CaseFile::getBoundingBox()
{
    return boundingboxarr;
}

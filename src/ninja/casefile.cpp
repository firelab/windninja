#include "casefile.h"

std::mutex zipMutex;


CaseFile::CaseFile()
{
    isZipOpen = false;
    caseZipFile = "";

    downloadedfromdem = false;
    elevsource = "";
}


void CaseFile::setIsZipOpen(bool isZippOpen)
{
    isZipOpen = isZippOpen;
}

bool CaseFile::getIsZipOpen()
{
    return isZipOpen;
}

void CaseFile::setCaseZipFile(std::string caseZippFile)
{
    caseZipFile = caseZippFile;
}

std::string CaseFile::getCaseZipFile()
{
    return caseZipFile;
}

// to avoid renaming the casefile except for the first run/ninja, checking for a specific starting zip file name
void CaseFile::renameCaseZipFile(std::string newCaseZipFile)
{
    if (parse("file", caseZipFile) == "tmp.ninja")
    {
        if (VSIRename(caseZipFile.c_str(), newCaseZipFile.c_str()) == 0)
        {
            CPLDebug("ZIP_RENAME", "Successfully renamed %s to %s", caseZipFile.c_str(), newCaseZipFile.c_str());
            caseZipFile = newCaseZipFile;
        } else
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to rename %s to %s", caseZipFile.c_str(), newCaseZipFile.c_str());
        }
    }
}


void CaseFile::addFileToZip(const std::string& caseZippFile, const std::string& withinZipPathedFilename, const std::string& fileToAdd)
{
    std::lock_guard<std::mutex> lock(zipMutex); // for multithreading issue

    try {
        bool doesZipExist = CPLCheckForFile(caseZippFile.c_str(), NULL);

        if (doesZipExist)
        {
            std::ifstream infile(caseZippFile);
            if (!infile.good())
            {
                CPLDebug("ZIP", "ZIP file does not exist: %s", caseZippFile.c_str());
                return;
            }
        }

        zipFile zip;
        if (!doesZipExist)
        {
            zip = cpl_zipOpen(caseZippFile.c_str(), APPEND_STATUS_CREATE);
        } else
        {
            zip = cpl_zipOpen(caseZippFile.c_str(), APPEND_STATUS_ADDINZIP);
        }

        if (zip == NULL)
        {
            CPLDebug("ZIP", "Could not open ZIP: %s", caseZippFile.c_str());
            return;
        }

        zip_fileinfo zi = {0};
        if (cpl_zipOpenNewFileInZip(zip, withinZipPathedFilename.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
        {
            CPLDebug("ZIP", "Could not open new file in ZIP: %s", withinZipPathedFilename.c_str());
            cpl_zipClose(zip, nullptr);
            return;
        }

        VSILFILE *file = VSIFOpenL(fileToAdd.c_str(), "rb");
        if (file == nullptr)
        {
            CPLDebug("VSIL", "Could not open file for reading with VSIL: %s", fileToAdd.c_str());
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
            CPLDebug("FileRead", "Failed to read file contents: %s", withinZipPathedFilename.c_str());
            CPLFree(data);
            VSIFCloseL(file);
            cpl_zipCloseFileInZip(zip);
            cpl_zipClose(zip, nullptr);
            return;
        }

        if (cpl_zipWriteInFileInZip(zip, data, static_cast<unsigned int>(fileSize)) != ZIP_OK)
        {
            CPLDebug("ZIP", "Error writing data to ZIP file: %s", withinZipPathedFilename.c_str());
        }

        CPLFree(data);
        VSIFCloseL(file);
        cpl_zipCloseFileInZip(zip);

        if (cpl_zipClose(zip, nullptr) != ZIP_OK)
        {
            CPLDebug("ZIP", "Error closing ZIP file: %s", caseZippFile.c_str());
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

std::string CaseFile::convertDateTime(const boost::local_time::local_date_time& ninjaTime)
{
    return "";
}

bool CaseFile::lookForDate(const std::string& date)
{
    return false;
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


void CaseFile::setDownloadedFromDem(bool downloadedfromdemm)
{
    downloadedfromdem = downloadedfromdemm;
}

bool CaseFile::getDownloadedFromDem()
{
    return downloadedfromdem;
}

void CaseFile::setElevSource(std::string elevsourcee)
{
    elevsource = elevsourcee;
}

std::string CaseFile::getElevSource()
{
    return elevsource;
}

void CaseFile::setBoundingBox(std::vector<double> boundingboxarrr)
{
    boundingboxarr = boundingboxarrr;
}

std::vector<double> CaseFile::getBoundingBox()
{
    return boundingboxarr;
}

void CaseFile::setWxTimes(std::vector<boost::local_time::local_date_time> timeList)
{
    timesForWx = timeList;
}

std::vector<boost::local_time::local_date_time> CaseFile::getWxTimes()
{
    return timesForWx;
}

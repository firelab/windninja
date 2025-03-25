#include "casefile.h"

std::mutex zipMutex;

CaseFile::CaseFile()
{
    isZipOpen = false;
    caseZipFile = "";
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
    if (strcmp( CPLGetFilename( caseZipFile.c_str() ), "tmp.ninja" ) == 0)
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

void CaseFile::addFileToZip(const std::string& withinZipPathedFilename, const std::string& fileToAdd)
{
    std::lock_guard<std::mutex> lock(zipMutex); // for multithreading issue

    try {
        bool doesZipExist = CPLCheckForFile(caseZipFile.c_str(), NULL);

        if (doesZipExist)
        {
            std::ifstream infile(caseZipFile);
            if (!infile.good())
            {
                CPLError(CE_Failure, CPLE_FileIO, "ZIP file does not exist: %s", caseZipFile.c_str());
                return;
            }
        }

        zipFile zip;
        if (!doesZipExist)
        {
            zip = cpl_zipOpen(caseZipFile.c_str(), APPEND_STATUS_CREATE);
        } else
        {
            zip = cpl_zipOpen(caseZipFile.c_str(), APPEND_STATUS_ADDINZIP);
        }

        if (zip == NULL)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Could not open ZIP: %s", caseZipFile.c_str());
            return;
        }

        zip_fileinfo zi = {0};
        if (cpl_zipOpenNewFileInZip(zip, withinZipPathedFilename.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Could not open new file in ZIP: %s", withinZipPathedFilename.c_str());
            cpl_zipClose(zip, nullptr);
            return;
        }

        VSILFILE *file = VSIFOpenL(fileToAdd.c_str(), "rb");
        if (file == nullptr)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Could not open file for reading with VSIL: %s", fileToAdd.c_str());
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
            CPLError(CE_Failure, CPLE_FileIO, "Failed to allocate memory for file data.");
            VSIFCloseL(file);
            cpl_zipCloseFileInZip(zip);
            cpl_zipClose(zip, nullptr);
            return;
        }

        if (VSIFReadL(data, 1, fileSize, file) != fileSize)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to read file contents: %s", withinZipPathedFilename.c_str());
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
            CPLDebug("ZIP", "Error closing ZIP file: %s", caseZipFile.c_str());
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

std::string CaseFile::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm* local_tm = std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d_%H-%M-%S");

    return oss.str();
}

std::string CaseFile::convertDateTimeToStd(const boost::local_time::local_date_time& ninjaTime)
{
    boost::local_time::local_time_facet* facet;
    facet = new boost::local_time::local_time_facet();
    facet->format("%Y-%m-%d_%H-%M-%S");

    std::ostringstream oss;
    oss.imbue( std::locale(std::locale::classic(), facet) );
    oss << ninjaTime;

    return oss.str();
}

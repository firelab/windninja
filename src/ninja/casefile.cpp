#include "casefile.h"

CaseFile::CaseFile()
{
    caseZipFile = "";
    finalCaseZipFile = "";
    isZipOpen = false;
    zipHandle = NULL;
}

void CaseFile::setCaseZipFile(std::string caseZippFile)
{
    if (caseZipFile != "")
    {
        throw std::runtime_error("not allowed to run setCaseZipFile() twice on the same CaseFile instance!!!");
    }

    caseZipFile = caseZippFile;
    finalCaseZipFile = caseZippFile;
}

void CaseFile::updateCaseZipFile(std::string newCaseZipFile)
{
    if (caseZipFile == "")
    {
        throw std::runtime_error("updateCaseZipFile() called before setCaseZipFile()!!!");
    }

    // only updates the first time that there is a difference, use the first input newCaseZipFile instance for the final caseZipFile name
    // this should only occur for the first run/ninja
    if (strcmp( caseZipFile.c_str(), finalCaseZipFile.c_str() ) == 0)
    {
        finalCaseZipFile = newCaseZipFile;
    }
}

void CaseFile::renameCaseZipFile()
{
    if (isZipOpen == true)
    {
        throw std::runtime_error("renameCaseZipFile() called on a still open zip file!!!");
    }

    if (strcmp( caseZipFile.c_str(), finalCaseZipFile.c_str() ) != 0)
    {
        if (VSIRename(caseZipFile.c_str(), finalCaseZipFile.c_str()) == 0)
        {
            CPLDebug("NINJA", "Successfully renamed %s to %s", caseZipFile.c_str(), finalCaseZipFile.c_str());
            caseZipFile = finalCaseZipFile;
        } else
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to rename %s to %s", caseZipFile.c_str(), finalCaseZipFile.c_str());
        }
    }
}

void CaseFile::openCaseZipFile()
{
    CPLDebug("NINJA", "opening case zip file %s", caseZipFile.c_str());

    if (isZipOpen == true)
    {
        throw std::runtime_error("openCaseZipFile() called on already open zip file!!! " + caseZipFile);
    }

    bool doesZipExist = CPLCheckForFile((char*)caseZipFile.c_str(), NULL);
    if (doesZipExist == true)
    {
        printf("WARNING: zip file %s already exists, replacing zip\n", caseZipFile.c_str());
        VSIUnlink( caseZipFile.c_str() );
    }

    zipHandle = CPLCreateZip(caseZipFile.c_str(), NULL);
    if (zipHandle == NULL)
    {
        throw std::runtime_error("Failed to create or open zip file!!! " + caseZipFile);
    }
    isZipOpen = true;
}

void CaseFile::closeCaseZipFile()
{
    // just skip if called on an unopened zip file, makes shutting WindNinja down unexpectedly easier to do
    if (isZipOpen == true)
    {
        CPLDebug("NINJA", "closing case zip file %s", caseZipFile.c_str());
        CPLCloseZip(zipHandle);
        zipHandle = NULL;
        isZipOpen = false;
    }
}

void CaseFile::addFileToZip(const std::string& zipEntry, const std::string& fileToAdd)
{
    // Acquire a lock for the multithreading issue, to protect the non-thread safe zip read and write process
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    try {
        bool doesZipExist = CPLCheckForFile((char*)caseZipFile.c_str(), NULL);

        if (doesZipExist)
        {
            std::ifstream infile(caseZipFile);
            if (!infile.good())
            {
                CPLError(CE_Failure, CPLE_FileIO, "ZIP file REALLY does not exist: %s", caseZipFile.c_str());
                return;
            }
        } else
        {
            CPLError(CE_Failure, CPLE_FileIO, "ZIP file does not exist: %s", caseZipFile.c_str());
            return;
        }

        if (zipHandle == NULL)
        {
            CPLError(CE_Failure, CPLE_FileIO, "tried to add file to unopened ZIP: %s", caseZipFile.c_str());
            return;
        }

        // read in the file data to be copied to the zip
        VSILFILE *FILE = VSIFOpenL(fileToAdd.c_str(), "rb");
        if (FILE == nullptr)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Could not open file for reading with VSIL: %s", fileToAdd.c_str());
            return;
        }

        VSIFSeekL(FILE, 0, SEEK_END);
        vsi_l_offset fileSize = VSIFTellL(FILE);
        VSIFSeekL(FILE, 0, SEEK_SET);  // rather than VSIRewindL(FILE);?

        char *data = (char*)CPLMalloc(fileSize);
        if (data == nullptr)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to allocate memory for file data.");
            VSIFCloseL(FILE);
            return;
        }

        if (VSIFReadL(data, 1, fileSize, FILE) != fileSize)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to read file contents: %s", fileToAdd.c_str());
            CPLFree(data);
            VSIFCloseL(FILE);
            return;
        }

        VSIFCloseL(FILE);

        // add the file data to the zip
        if (CPLCreateFileInZip(zipHandle, zipEntry.c_str(), NULL) != CE_None)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to create file in zip: %s", zipEntry.c_str());
            CPLFree(data);
            return;
        }

        if (CPLWriteFileInZip(zipHandle, data, static_cast<int>(fileSize)) != CE_None)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to write data to file in zip: %s", zipEntry.c_str());
            CPLFree(data);
            return;
        }

        if (CPLCloseFileInZip(zipHandle) != CE_None)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to close file in zip: %s", zipEntry.c_str());
            CPLFree(data);
            return;
        }

        CPLFree(data);

    } catch (const std::exception& e)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Exception caught during casefile addFileToZip(): %s", e.what());
    } catch (...)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Exception caught during casefile addFileToZip(): Cannot determine exception type.");
    }
}

bool CaseFile::getIsZipOpen()
{
    return isZipOpen;
}

std::string CaseFile::getCaseZipFile()
{
    return caseZipFile;
}

std::string CaseFile::getCurrentTime()
{
    const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    boost::posix_time::time_facet* facet;
    facet = new boost::posix_time::time_facet();
    facet->format("%Y-%m-%d_%H-%M-%S");

    std::ostringstream oss;
    oss.imbue( std::locale(std::locale::classic(), facet) );
    oss << now;

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

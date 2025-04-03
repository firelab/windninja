#include "casefile.h"

std::mutex zipMutex;

CaseFile::CaseFile()
{
    caseZipFile = "";
    finalCaseZipFile = "";
    isZipOpen = false;
    zipHandle = NULL;

    setCaseZipFileCount = 0;
}

void CaseFile::setCaseZipFile(std::string caseZippFile)
{
    if (setCaseZipFileCount > 0)
    {
        CPLError(CE_Failure, CPLE_FileIO, "not allowed to run setCaseZipFile() twice on the same CaseFile instance!!!");
    }

    caseZipFile = caseZippFile;
    finalCaseZipFile = caseZippFile;
    setCaseZipFileCount++;
}

void CaseFile::updateCaseZipFile(std::string newCaseZipFile)
{
    if (setCaseZipFileCount == 0)
    {
        CPLError(CE_Failure, CPLE_FileIO, "updateCaseZipFile() called before setCaseZipFile()!!!");
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
        CPLError(CE_Failure, CPLE_FileIO, "renameCaseZipFile() called on a still open zip file: %s", caseZipFile.c_str());
    }

    if (strcmp( caseZipFile.c_str(), finalCaseZipFile.c_str() ) != 0)
    {
        if (VSIRename(caseZipFile.c_str(), finalCaseZipFile.c_str()) == 0)
        {
            //CPLDebug("ZIP_RENAME", "Successfully renamed %s to %s", caseZipFile.c_str(), finalCaseZipFile.c_str());
            printf("ZIP_RENAME: Successfully renamed %s to %s\n", caseZipFile.c_str(), finalCaseZipFile.c_str());
            caseZipFile = finalCaseZipFile;
        } else
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to rename %s to %s", caseZipFile.c_str(), finalCaseZipFile.c_str());
        }
    }
}

void CaseFile::openCaseZipFile()
{
    std::cout << "openingCaseZipFile" << std::endl;

    if (isZipOpen == true)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Running openCaseZipFile() on already open zip file: %s", caseZipFile.c_str());
    }

    bool doesZipExist = CPLCheckForFile((char*)caseZipFile.c_str(), NULL);
    if (doesZipExist == true)
    {
        printf("warning: zip file %s already exists, replacing zip", caseZipFile.c_str());
        VSIUnlink( caseZipFile.c_str() );
    }

    zipHandle = CPLCreateZip(caseZipFile.c_str(), NULL);
    if (zipHandle == NULL)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Failed to create or open zip file: %s", caseZipFile.c_str());
    }
    isZipOpen = true;
}

void CaseFile::closeCaseZipFile()
{
    std::cout << "closingCaseZipFile" << std::endl;

    if (isZipOpen == false)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Running closeCaseZipFile() on an unopened zip file: %s", caseZipFile.c_str());
    }

    CPLCloseZip(zipHandle);
    zipHandle = NULL;
    isZipOpen = false;
}

void CaseFile::addFileToZip(const std::string& withinZipPathedFilename, const std::string& fileToAdd)
{
    std::lock_guard<std::mutex> lock(zipMutex); // for multithreading issue

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
        VSILFILE *file = VSIFOpenL(fileToAdd.c_str(), "rb");
        if (file == nullptr)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Could not open file for reading with VSIL: %s", fileToAdd.c_str());
            closeCaseZipFile();
            return;
        }

        VSIFSeekL(file, 0, SEEK_END);
        vsi_l_offset fileSize = VSIFTellL(file);
        VSIFSeekL(file, 0, SEEK_SET);  // rather than VSIRewindL(file);?

        char *data = (char*)CPLMalloc(fileSize);
        if (data == nullptr)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to allocate memory for file data.");
            VSIFCloseL(file);
            closeCaseZipFile();
            return;
        }

        if (VSIFReadL(data, 1, fileSize, file) != fileSize)
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to read file contents: %s", fileToAdd.c_str());
            CPLFree(data);
            VSIFCloseL(file);
            closeCaseZipFile();
            return;
        }

        VSIFCloseL(file);

        // add the file data to the zip
        if (CPLCreateFileInZip(zipHandle, withinZipPathedFilename.c_str(), NULL) != CE_None) {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to create file in zip: %s", withinZipPathedFilename.c_str());
            CPLFree(data);
            CPLCloseZip(zipHandle);
            return;
        }

        if (CPLWriteFileInZip(zipHandle, data, static_cast<int>(fileSize)) != CE_None) {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to write data to file in zip: %s", withinZipPathedFilename.c_str());
            CPLFree(data);
            CPLCloseZip(zipHandle);
        }

        if (CPLCloseFileInZip(zipHandle) != CE_None) {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to close file in zip: %s", withinZipPathedFilename.c_str());
            CPLFree(data);
            CPLCloseZip(zipHandle);
        }

        CPLFree(data);

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

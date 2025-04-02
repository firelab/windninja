#include "casefile.h"

std::mutex zipMutex;

CaseFile::CaseFile()
{
    isZipOpen = false;
    caseZipFile = "";
    zipHandle = NULL;
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
        //closeCaseZipFile();
        //std::replace(caseZipFile.begin(),caseZipFile.end(), '\\', '/');
        //std::replace(newCaseZipFile.begin(),newCaseZipFile.end(), '\\', '/');
        if (VSIRename(caseZipFile.c_str(), newCaseZipFile.c_str()) == 0)
        {
            CPLDebug("ZIP_RENAME", "Successfully renamed %s to %s", caseZipFile.c_str(), newCaseZipFile.c_str());
            //printf("ZIP_RENAME: Successfully renamed %s to %s\n", caseZipFile.c_str(), newCaseZipFile.c_str());
            caseZipFile = newCaseZipFile;
        } else
        {
            CPLError(CE_Failure, CPLE_FileIO, "Failed to rename %s to %s", caseZipFile.c_str(), newCaseZipFile.c_str());
        }
        //openCaseZipFile();
    }
}

void CaseFile::openCaseZipFile()
{
    //bool doesZipExist = CPLCheckForFile((char*)caseZipFile.c_str(), NULL);
std::cout << "openingCaseZipFile" << std::endl;
    zipHandle = CPLCreateZip(caseZipFile.c_str(), NULL);
    if (zipHandle == NULL)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Failed to create or open zip file: %s", caseZipFile.c_str());
    }
}

void CaseFile::closeCaseZipFile()
{
std::cout << "closingCaseZipFile" << std::endl;
    CPLCloseZip(zipHandle);
    zipHandle = NULL;
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

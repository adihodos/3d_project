#include "pch_hdr.h"
#include "scoped_ptr.h"
#include "ifs_model_loader.h"
#include "win32_traits.h"

bool 
base::model_format_handlers::IFSLoader::readStringData(
    HANDLE fptr, 
    std::vector<unsigned char>* str
    ) 
{
    unsigned int strLen;
    DWORD bytesOut;
    if (!::ReadFile(fptr, &strLen, sizeof(strLen), &bytesOut, 0))
        return false;

    str->resize(strLen);
    return ::ReadFile(fptr, &(*str)[0], strLen, &bytesOut, 0) == TRUE;
}

bool 
base::model_format_handlers::IFSLoader::readFloat(
    HANDLE fptr, 
    float* outVal
    ) 
{
    DWORD bytesRead;
    return ::ReadFile(fptr, outVal, sizeof(float), &bytesRead, 0);
}

bool 
base::model_format_handlers::IFSLoader::readUint(
    HANDLE fptr, 
    unsigned int* outVal
    ) 
{
    DWORD bytesRead;
    return ::ReadFile(fptr, outVal, sizeof(unsigned int), &bytesRead, 0) == TRUE;
}

bool 
base::model_format_handlers::IFSLoader::readElementCount(
    HANDLE fptr, 
    unsigned int* vertexCnt, 
    const char* hdrString
    ) 
{
    std::vector<unsigned char> headerStr;
    if (!readStringData(fptr, &headerStr))
        return false;

    std::string vertexHDRStr((const char*)&headerStr[0]);
    if (vertexHDRStr.compare(hdrString))
        return false;

    return readUint(fptr, vertexCnt);
}

bool 
base::model_format_handlers::IFSLoader::readVertices(
    HANDLE fptr, 
    unsigned int howMany
    ) 
{
    vertexData_.reserve(howMany);
    bool failed = true;
    for (unsigned int i = 0; (i < howMany) && failed; ++i) {
        gfx::vector3F currentVertex;
        failed = readFloat(fptr, &currentVertex.x_);
        failed = readFloat(fptr, &currentVertex.y_);
        failed = readFloat(fptr, &currentVertex.z_);
        currentVertex.z_ *= 1.0f;

        vertexData_.push_back(currentVertex);
    }
    return failed;
}

bool 
base::model_format_handlers::IFSLoader::readFaces(
    HANDLE fptr, 
    unsigned int howMany
    ) 
{
    indexData_.reserve(howMany);
    bool succeeded = true;
    unsigned int currentIndex;
    for (unsigned int i = 0; (i < howMany) && succeeded; ++i) {
        succeeded = readUint(fptr, &currentIndex);
        indexData_.push_back(currentIndex);
        succeeded = readUint(fptr, &currentIndex);
        indexData_.push_back(currentIndex);
        succeeded = readUint(fptr, &currentIndex);
        indexData_.push_back(currentIndex);
    }

    return succeeded;
}

bool 
base::model_format_handlers::IFSLoader::readIFSHeader(
    HANDLE fptr
    ) 
{
    std::vector<unsigned char> strBuff;
    if (!readStringData(fptr, &strBuff))
        return false;

    std::string readStr((const char*) &strBuff[0]);
    if (readStr.compare("IFS"))
        return false;

    if (!readFloat(fptr, &ifsVersion_))
        return false;

    if (!readStringData(fptr, &strBuff))
        return false;

    modelName_.assign((const char*) &strBuff[0]);
    return true;
}

bool
base::model_format_handlers::IFSLoader::loadModel(
    const char* modelFile
    ) 
{
    isValid_ = false;

    std::unique_ptr<void, win32::traits::file_deleter> fileFD(
        ::CreateFileA(modelFile, GENERIC_READ, 0, 0, OPEN_EXISTING, 
                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0));

    if (fileFD.get() == INVALID_HANDLE_VALUE)
        return false;

    if (!readIFSHeader(fileFD.get()))
        return false;

    //
    // read vertex count
    unsigned int elements;
    if (!readElementCount(fileFD.get(), &elements, "VERTICES"))
        return false;

    if (!readVertices(fileFD.get(), elements))
        return false;

    //
    // read face count
    if (!readElementCount(fileFD.get(), &numFaces_, "TRIANGLES"))
        return false;

    if (!readFaces(fileFD.get(), numFaces_))
        return false;

    isValid_ = true;
    return true;
}
#pragma once

#include <string>
#include <vector>
#include <gfx/vector3.h>
#include "misc.h"

namespace base {

namespace model_format_handlers {

class IFSLoader {
private :
    float                       ifsVersion_;
    std::string                 modelName_;
    std::vector<gfx::vector3F>  vertexData_;
    std::vector<unsigned int>   indexData_;
    unsigned int                numFaces_;
    bool                        isValid_;

    bool readStringData(HANDLE fptr, std::vector<unsigned char>* str);

    bool readFloat(HANDLE fptr, float* outVal);

    bool readUint(HANDLE fptr, unsigned int* outVal);

    bool readElementCount(HANDLE fptr, unsigned int* vertexCnt, const char* hdrString);

    bool readVertices(HANDLE fptr, unsigned int howMany);

    bool readFaces(HANDLE fptr, unsigned int howMany);

    bool readIFSHeader(HANDLE fptr);

public :
    IFSLoader() : isValid_(false) {}

    bool loadModel(const char* modelFile);

    const gfx::vector3F* getVertexListPointer() const {
        assert(isValid_);
        return &vertexData_[0];
    }

    size_t getVertexCount() const {
        assert(isValid_);
        return vertexData_.size();
    }

    const unsigned int* getIndexListPointer() const {
        assert(isValid_);
        return &indexData_[0];
    }

    size_t getIndexCount() const {
        assert(isValid_);
        return indexData_.size();
    }

    size_t getFaceCount() const {
        assert(isValid_);
        return numFaces_;
    }
};

} // namespace model_format_handlers

} // namespace base
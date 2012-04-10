#pragma once

#include "misc.h"
#include "scoped_ptr.h"

namespace test {

class simple_object {
private :
    NO_CC_ASSIGN(simple_object);

    struct implementation_details;
    base::scoped_ptr<implementation_details> impl_;
public :
    simple_object();

    ~simple_object();

    void initialize();

    void draw();
};

}
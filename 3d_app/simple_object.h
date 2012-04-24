#pragma once

#include <gfx/matrix4X4.h>
#include "misc.h"
#include "scoped_ptr.h"

namespace game {
  class renderer;
}

namespace test {

class simple_object {
private :
    NO_CC_ASSIGN(simple_object);

    struct implementation_details;
    base::scoped_ptr<implementation_details> impl_;
public :
    simple_object();

    ~simple_object();

    void initialize(game::renderer* r);

    void draw(game::renderer*, const gfx::matrix_4X4F&);
};

}
#pragma once

#include "lazy_singleton.h"
#include "misc.h"
#include "scoped_ptr.h"

namespace game {

class scene : public base::singleton_life_traits<scene> {
private :
    NO_CC_ASSIGN(scene);
    friend class base::singleton_life_traits<scene>;

    struct implementation_details;
    base::scoped_ptr<implementation_details> impl_;

    scene();

    ~scene();
public :
    void draw();
};

typedef base::lazy_singleton<scene> scene_instance_t;

} // namespace game
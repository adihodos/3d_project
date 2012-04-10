#include "pch_hdr.h"
#include "d3d_renderer.h"
#include "scene.h"

struct game::scene::implementation_details {
};

game::scene::scene() : impl_(new implementation_details) {}

game::scene::~scene() {}

void game::scene::draw() {
    
}
#pragma once

#include <gfx/matrix4X4.h>
#include <list>
#include "interface_drawable.h"

namespace game {

class IDrawable;

class scene_graph {
private :
  struct graph_node_t {
    IDrawable*        drawable_object_;
    const int         node_id_;

    graph_node_t(int node_id) : node_id_(node_id_) {}

    void process_node(const gfx::matrix_4X4F& transform) {
      drawable_object_->draw(transform);
    }
  };

  std::list<graph_node_t*>  graph_nodes_;

public :
  scene_graph() {}
};

}
#pragma once

#include <gfx/matrix4X4.h>

namespace game {

class IDrawable {
public :
  virtual ~IDrawable() {}

  virtual void draw(const gfx::matrix_4X4F&) = 0;
};

}
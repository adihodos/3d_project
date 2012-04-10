#pragma once

#include <cassert>
#include "misc.h"

namespace base {

template<typename ptr_type, 
         template<typename> class ptr_policy> class scoped_ptr;

template<typename ptr_type, template<typename> class ptr_policy>
ptr_type* scoped_ptr_get(const scoped_ptr<ptr_type, ptr_policy>&);

template<typename ptr_type, template<typename> class ptr_policy>
ptr_type* scoped_ptr_release(scoped_ptr<ptr_type, ptr_policy>&);

template<typename ptr_type, template<typename> class ptr_policy>
void scoped_ptr_reset(scoped_ptr<ptr_type, ptr_policy>&, ptr_type*);

template<typename ptr_type, template<typename> class ptr_policy>
ptr_type** scoped_ptr_get_ptr_ptr(scoped_ptr<ptr_type, ptr_policy>&);

template<typename ptr_type, template<typename> class ptr_policy>
void swap(scoped_ptr<ptr_type, ptr_policy>&, scoped_ptr<ptr_type, ptr_policy>&);

template<typename T>
struct ptr_defpol {
  static void dispose(T* ptr) {
    delete ptr;
  }
};

template<typename T>
struct array_access {
  static T& at(T* ptr, int index) {
    return *(ptr + index);
  }

  static const T& at(const T* ptr, int index) {
    return *(ptr + index);
  }
};

template<typename T>
struct ptr_array : public array_access<T> {
  static void dispose(T* ptr) {
    delete[] ptr;
  }
};

template<typename ptr_type, 
         template<typename> class ptr_policy = ptr_defpol
> class scoped_ptr {
public :
  typedef ptr_type                  value;
  typedef ptr_type*                 pointer;
  typedef ptr_type**                pointer_pointer;
  typedef const ptr_type*           const_pointer;
  typedef ptr_type&                 reference;
  typedef const ptr_type&           const_reference;
  typedef ptr_policy<ptr_type>      usage_policy;

  scoped_ptr() : owned_ptr_(nullptr) {}

  explicit scoped_ptr(ptr_type* ptr) : owned_ptr_(ptr) {}

  scoped_ptr(scoped_ptr<ptr_type, ptr_policy>&& other) : owned_ptr_(nullptr) {
      steal_from_other(std::forward<scoped_ptr<ptr_type, ptr_policy>&&>(other));
  }

  ~scoped_ptr() {
    usage_policy::dispose(owned_ptr_);
  }

  scoped_ptr<ptr_type, ptr_policy>& 
  operator=(scoped_ptr<ptr_type, ptr_policy>&& other) {
      if (this != &other) {
          steal_from_other(
              std::forward<scoped_ptr<ptr_type, ptr_policy>&&>(other));
      }
      return *this;
  }

  bool operator!() const {
    return owned_ptr_ == nullptr;
  }

  pointer operator->() const {
    assert(owned_ptr_);
    return owned_ptr_;
  }

  reference operator*() {
    assert(owned_ptr_);
    return *owned_ptr_;
  }

  const_reference operator*() const {
    assert(owned_ptr_);
    return *owned_ptr_;
  }

  reference operator[](int idx) {
    assert(owned_ptr_);
    return usage_policy::at(owned_ptr_, idx);
  }

  const_reference operator[](int idx) const {
    assert(owned_ptr_);
    return usage_policy::at(owned_ptr_, idx);
  }

private :
  friend pointer scoped_ptr_get<>(const scoped_ptr<ptr_type, ptr_policy>&);
  friend pointer scoped_ptr_release<>(scoped_ptr<ptr_type, ptr_policy>&);
  friend void scoped_ptr_reset<>(scoped_ptr<ptr_type, ptr_policy>&, ptr_type*);
  friend pointer_pointer scoped_ptr_get_ptr_ptr<>(scoped_ptr<ptr_type, ptr_policy>&);
  friend void swap<>(scoped_ptr<ptr_type, ptr_policy>&, scoped_ptr<ptr_type, ptr_policy>&);

  ptr_type* internal_get() const {
    return owned_ptr_;
  }

  ptr_type* internal_release() {
    ptr_type* old(owned_ptr_);
    owned_ptr_ = nullptr;
    return old;
  }

  void internal_reset(ptr_type* nptr) {
    if (nptr != owned_ptr_) {
      usage_policy::dispose(owned_ptr_);
      owned_ptr_ = nptr;
    }
  }

  template<typename convertible>
  void internal_reset(convertible* other) {
    if (other != owned_ptr_) {
      usage_policy::dispose(owned_ptr_);
      owned_ptr_ = other;
    }
  }

  pointer_pointer internal_get_ptrptr() {
    assert(owned_ptr_ == nullptr);
    return &owned_ptr_;
  }

  void internal_swap(scoped_ptr<ptr_type, ptr_policy>& other) {
    ptr_type* tmp(owned_ptr_);
    owned_ptr_ = other.owned_ptr_;
    other.owned_ptr_ = tmp;
  }

  void steal_from_other(scoped_ptr<ptr_type, ptr_policy>&& other) {
      usage_policy::dispose(owned_ptr_);
      owned_ptr_ = other.owned_ptr_;
      other.owned_ptr_ = nullptr;
  }

  pointer                             owned_ptr_;
  NO_CC_ASSIGN(scoped_ptr);
};

template<typename ptr_type, template<typename> class ptr_policy>
inline
ptr_type* scoped_ptr_get(const scoped_ptr<ptr_type, ptr_policy>& sp) {
  return sp.internal_get();
}

template<typename ptr_type, template<typename> class ptr_policy>
ptr_type* scoped_ptr_release(scoped_ptr<ptr_type, ptr_policy>& sp) {
  return sp.internal_release();
}

template<typename ptr_type, template<typename> class ptr_policy>
void 
scoped_ptr_reset(
  scoped_ptr<ptr_type, ptr_policy>& sp, 
  ptr_type* nptr = nullptr
  ) 
{
  sp.internal_reset(nptr);
}

template<typename ptr_type, template<typename> class ptr_policy>
ptr_type** 
scoped_ptr_get_ptr_ptr(scoped_ptr<ptr_type, ptr_policy>& sp) {
  return sp.internal_get_ptrptr();
}

template<typename ptr_type, template<typename> class ptr_policy>
void 
swap(
  scoped_ptr<ptr_type, ptr_policy>& lhs, 
  scoped_ptr<ptr_type, ptr_policy>& rhs
  ) 
{
  lhs.internal_swap(rhs);
}

}
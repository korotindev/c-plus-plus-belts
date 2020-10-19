#include "comparators.h"

#include <functional>
#include <optional>
#include <sstream>

#include "object.h"
#include "object_holder.h"

using namespace std;

namespace Runtime {

  bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
    if (auto lhs_ptr = lhs.TryAs<Runtime::Number>()) {
      auto rhs_ptr = rhs.TryAs<Runtime::Number>();
      if (rhs_ptr) {
        return lhs_ptr->GetValue() == rhs_ptr->GetValue();
      } else {
        throw Runtime::Error("bad numbers comparison");
      }
    } else if (auto lhs_ptr = lhs.TryAs<Runtime::String>()) {
      auto rhs_ptr = rhs.TryAs<Runtime::String>();
      if (rhs_ptr) {
        return lhs_ptr->GetValue() == rhs_ptr->GetValue();
      } else {
        throw Runtime::Error("bad strings comparison");
      }
    } else {
      throw Runtime::Error("bad comparison");
    }
  }

  bool Less(ObjectHolder lhs, ObjectHolder rhs) {
    if (auto lhs_ptr = lhs.TryAs<Runtime::Number>()) {
      auto rhs_ptr = rhs.TryAs<Runtime::Number>();
      if (rhs_ptr) {
        return lhs_ptr->GetValue() < rhs_ptr->GetValue();
      } else {
        throw Runtime::Error("bad numbers comparison");
      }
    } else if (auto lhs_ptr = lhs.TryAs<Runtime::String>()) {
      auto rhs_ptr = rhs.TryAs<Runtime::String>();
      if (rhs_ptr) {
        return lhs_ptr->GetValue() < rhs_ptr->GetValue();
      } else {
        throw Runtime::Error("bad strings comparison");
      }
    } else {
      throw Runtime::Error("bad comparison");
    }
  }

} /* namespace Runtime */

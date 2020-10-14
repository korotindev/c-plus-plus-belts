#include "object.h"

#include <sstream>
#include <string_view>

#include "statement.h"

using namespace std;

namespace Runtime {

  void ClassInstance::Print(std::ostream& os) {}

  bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
    return cls_.HasMethod(method, argument_count);
  }

  const Closure& ClassInstance::Fields() const { return closure_; }

  Closure& ClassInstance::Fields() { return closure_; }

  ClassInstance::ClassInstance(const Class& cls) : cls_(cls), closure_() {}

  ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
    // TODO: Process unknown method call ?
    const auto method_ptr = cls_.GetMethod(method);

    Closure method_closure;
    method_closure["self"] = ObjectHolder::Share(*this);

    const auto& method_formal_params = method_ptr->formal_params;
    // TODO: Process wrong arguments count ?
    for (size_t arg_id = 0; arg_id < method_formal_params.size(); arg_id++) {
      method_closure[method_formal_params.at(arg_id)] = actual_args.at(arg_id);
    }

    return method_ptr->body->Execute(method_closure);
  }

  Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
      : name_(move(name)), vtable_(), methods_(move(methods)), parent_(parent) {
    if (parent_) {
      vtable_ = parent->vtable_;
    }

    for (auto& method : methods_) {
      vtable_[method.name] = &method;
    }
  }

  const Method* Class::GetMethod(const std::string& name) const { return vtable_.at(name); }

  bool Class::HasMethod(const std::string& name, size_t argument_count) const {
    if (vtable_.count(name)) {
      return vtable_.at(name)->formal_params.size() == argument_count;
    }

    return false;
  }

  void Class::Print(ostream& os) { os << GetName(); }

  const std::string& Class::GetName() const { return name_; }

  void Bool::Print(std::ostream& os) {
    if (GetValue()) {
      os << "True";
    } else {
      os << "False";
    }
  }

} /* namespace Runtime */

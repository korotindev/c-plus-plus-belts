#include "object.h"

#include <sstream>
#include <string_view>

#include "statement.h"

using namespace std;

namespace Runtime {

  void ClassInstance::Print(std::ostream& os) {
    if (HasMethod("__str__", 0)) {
      auto res_oh = Call("__str__", {});
      if (res_oh) {
        res_oh->Print(os);
      }
    } else {
      os << this;
    }
  }

  bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
    return cls_.HasMethod(method, argument_count);
  }

  bool ClassInstance::HasSimilarMethod(const std::string& method) const { return cls_.GetMethod(method); }

  const Closure& ClassInstance::Fields() const { return closure_; }

  Closure& ClassInstance::Fields() { return closure_; }

  ClassInstance::ClassInstance(const Class& cls) : cls_(cls), closure_() {}

  ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
    // TODO: Process unknown method call ?
    const auto method_ptr = cls_.GetMethod(method);

    if (!method_ptr) {
      ostringstream os;
      os << "unknown method \"" << method << "\" called for ";
      this->Print(os);
      throw Runtime::Error(os.str());
    }

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

  const Method* Class::GetMethod(const std::string& name) const {
    if (const auto it = vtable_.find(name); it != vtable_.cend()) {
      return it->second;
    }

    return nullptr;
  }

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

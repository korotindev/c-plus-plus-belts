#pragma once

#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "object_holder.h"

namespace Ast {
  class Statement;
}

namespace Runtime {

  class Object {
   public:
    virtual ~Object() = default;
    virtual void Print(std::ostream& os) = 0;
  };

  template <typename T>
  class ValueObject : public Object {
   public:
    ValueObject(T v) : value(v) {}

    void Print(std::ostream& os) override { os << value; }

    const T& GetValue() const { return value; }

   private:
    T value;
  };

  using String = ValueObject<std::string>;
  using Number = ValueObject<int>;

  class Bool : public ValueObject<bool> {
   public:
    using ValueObject<bool>::ValueObject;
    void Print(std::ostream& os) override;
  };

  struct Method {
    std::string name;
    std::vector<std::string> formal_params;
    std::unique_ptr<Ast::Statement> body;
  };

  class Class : public Object {
    std::string name_;
    std::unordered_map<std::string, Method*> vtable_;
    std::vector<Method> methods_;
    const Class* parent_;

   public:
    explicit Class(std::string name, std::vector<Method> methods, const Class* parent);
    const Method* GetMethod(const std::string& name) const;
    bool HasMethod(const std::string& method, size_t argument_count) const;
    bool HasSimilarMethod(const std::string& method) const;
    const std::string& GetName() const;
    void Print(std::ostream& os) override;
  };

  class ClassInstance : public Object {
    const Class& cls_;
    Closure closure_;

   public:
    explicit ClassInstance(const Class& cls);

    void Print(std::ostream& os) override;

    ObjectHolder Call(const std::string& method, const std::vector<ObjectHolder>& actual_args);
    bool HasMethod(const std::string& method, size_t argument_count) const;
    bool HasSimilarMethod(const std::string& method) const;

    Closure& Fields();
    const Closure& Fields() const;
  };

  class Error : public std::runtime_error {
   public:
    Error(std::string msg) : std::runtime_error(move(msg)) {}
  };

}  // namespace Runtime

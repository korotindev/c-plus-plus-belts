#pragma once

#include <memory>
#include <unordered_map>

namespace Runtime {

  class Object;

  class ObjectHolder {
   public:
    ObjectHolder() = default;

    template <typename T>
    static ObjectHolder Own(T&& object) {
      return ObjectHolder(std::make_shared<T>(std::forward<T>(object)));
    }

    static ObjectHolder Share(Object& object);
    static ObjectHolder None();

    Object& operator*();
    const Object& operator*() const;
    Object* operator->();
    const Object* operator->() const;

    Object* Get();
    const Object* Get() const;

    template <typename T>
    T* TryAs() {
      return dynamic_cast<T*>(this->Get());
    }

    template <typename T>
    const T* TryAs() const {
      return dynamic_cast<const T*>(this->Get());
    }

    explicit operator bool() const;

    bool IsReturnable() const;
    void MakrReturnable();

   private:
    ObjectHolder(std::shared_ptr<Object> data) : data(std::move(data)), returnable(false) {}

    std::shared_ptr<Object> data;
    bool returnable = false;
  };

  using Closure = std::unordered_map<std::string, ObjectHolder>;

} /* namespace Runtime */

using ObjectHolder = Runtime::ObjectHolder;

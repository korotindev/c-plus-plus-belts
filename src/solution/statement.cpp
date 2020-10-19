#include "statement.h"

#include <iostream>
#include <sstream>

#include "object.h"

using namespace std;

namespace Ast {

  bool transform_object_holder_to_bool(ObjectHolder oh) {
    if (auto res_ptr = oh.TryAs<Runtime::Bool>()) {
      return res_ptr->GetValue();
    } else if (auto res_ptr = oh.TryAs<Runtime::Number>()) {
      return res_ptr->GetValue() != 0;
    } else if (auto res_ptr = oh.TryAs<Runtime::String>()) {
      return !(res_ptr->GetValue().empty());
    } else if (oh.TryAs<Runtime::ClassInstance>()) {
      return true;
    } else {
      return false;
    }
  }

  using Runtime::Closure;

  ObjectHolder Assignment::Execute(Closure& closure) {
    // TODO think about closure passing
    closure[var_name] = right_value->Execute(closure);
    return ObjectHolder::Share(*closure[var_name]);
  }

  Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_name(move(var)), right_value(move(rv)) {}

  VariableValue::VariableValue(std::string var_name) : dotted_ids({move(var_name)}) {}

  VariableValue::VariableValue(std::vector<std::string> dotted_ids_) : dotted_ids(move(dotted_ids_)) {}

  ObjectHolder VariableValue::Execute(Closure& closure) {
    // TODO: check for overall adequacy
    ObjectHolder oh;
    if (auto it = closure.find(dotted_ids[0]); it != closure.end()) {
      oh = it->second;
    } else {
      throw Runtime::Error("unknown literal: " + dotted_ids[0]);
    }

    for (size_t i = 1; i < dotted_ids.size(); i++) {
      // TODO: Check object for nullptr
      Runtime::ClassInstance* object = oh.TryAs<Runtime::ClassInstance>();
      const auto& id = dotted_ids[i];
      if (auto it = object->Fields().find(id); it != object->Fields().end()) {
        oh = it->second;
      } else if (object->HasSimilarMethod(id)) {
        oh = ObjectHolder::Share(*object);
        break;
      } else {
        throw Runtime::Error("unknown literal: " + id);
      }
    }

    return oh;
  }

  unique_ptr<Print> Print::Variable(std::string var) {
    return make_unique<Print>(make_unique<Ast::VariableValue>(var));
  }

  ostream* Print::output = &cout;

  Print::Print(unique_ptr<Statement> argument_) { args.emplace_back(move(argument_)); }

  Print::Print(vector<unique_ptr<Statement>> args) : args(move(args)) {}

  ObjectHolder Print::Execute(Closure& closure) {
    bool first = true;
    for (auto& arg : args) {
      if (!first) {
        *output << " ";
      }
      ObjectHolder oh = arg->Execute(closure);
      if (oh) {
        oh->Print(*output);
      } else {
        *output << "None";
      }
      first = false;
    }
    *output << '\n';
    return ObjectHolder::None();
  }

  void Print::SetOutputStream(ostream& output_stream) { output = &output_stream; }

  MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method,
                         std::vector<std::unique_ptr<Statement>> args)
      : object(move(object)), method(move(method)), args(move(args)) {}

  ObjectHolder MethodCall::Execute(Closure& closure) {
    // TODO: check obj for nullptr
    auto obj_oh = object->Execute(closure);
    auto obj_ptr = obj_oh.TryAs<Runtime::ClassInstance>();
    if (!obj_ptr) {
      ostringstream err_msg;
      obj_oh->Print(err_msg);
      err_msg << " is not an object";
      throw Runtime::Error(err_msg.str());
    }
    vector<Runtime::ObjectHolder> computed_args;
    transform(args.begin(), args.end(), back_inserter(computed_args),
              [&closure](unique_ptr<Statement>& arg) { return arg->Execute(closure); });

    return obj_ptr->Call(method, computed_args);
  }

  ObjectHolder Stringify::Execute(Closure& closure) {
    auto arg_oh = argument->Execute(closure);
    ostringstream res;
    arg_oh->Print(res);
    return ObjectHolder::Own(Runtime::String(res.str()));
  }

  ObjectHolder Add::Execute(Closure& closure) {
    auto lhs_oh = lhs->Execute(closure);
    auto rhs_oh = rhs->Execute(closure);

    if (auto lhs_ptr = lhs_oh.TryAs<Runtime::Number>()) {
      auto rhs_ptr = rhs_oh.TryAs<Runtime::Number>();
      if (lhs_ptr && rhs_ptr) {
        return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() + rhs_ptr->GetValue()));
      } else {
        throw Runtime::Error("bad numbers addition");
      }
    } else if (auto lhs_ptr = lhs_oh.TryAs<Runtime::String>()) {
      auto rhs_ptr = rhs_oh.TryAs<Runtime::String>();
      if (lhs_ptr && rhs_ptr) {
        return ObjectHolder::Own(Runtime::String(lhs_ptr->GetValue() + rhs_ptr->GetValue()));
      } else {
        throw Runtime::Error("bad strings addition");
      }
    } else if (auto lhs_ptr = lhs_oh.TryAs<Runtime::ClassInstance>()) {
      return lhs_ptr->Call("__add__", {rhs_oh});
    } else {
      throw Runtime::Error("bad arguments in add");
    }
  }

  ObjectHolder Sub::Execute(Closure& closure) {
    auto lhs_oh = lhs->Execute(closure);
    auto rhs_oh = rhs->Execute(closure);
    auto lhs_ptr = lhs_oh.TryAs<Runtime::Number>();
    auto rhs_ptr = rhs_oh.TryAs<Runtime::Number>();
    if (lhs_ptr && rhs_ptr) {
      return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() + rhs_ptr->GetValue()));
    } else {
      throw Runtime::Error("bad addition");
    }
    return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() - rhs_ptr->GetValue()));
  }

  ObjectHolder Mult::Execute(Runtime::Closure& closure) {
    auto lhs_oh = lhs->Execute(closure);
    auto rhs_oh = rhs->Execute(closure);
    auto lhs_ptr = lhs_oh.TryAs<Runtime::Number>();
    auto rhs_ptr = rhs_oh.TryAs<Runtime::Number>();
    if (lhs_ptr && rhs_ptr) {
      return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() + rhs_ptr->GetValue()));
    } else {
      throw Runtime::Error("bad addition");
    }
    return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() * rhs_ptr->GetValue()));
  }

  ObjectHolder Div::Execute(Runtime::Closure& closure) {
    auto lhs_oh = lhs->Execute(closure);
    auto rhs_oh = rhs->Execute(closure);
    auto lhs_ptr = lhs_oh.TryAs<Runtime::Number>();
    auto rhs_ptr = rhs_oh.TryAs<Runtime::Number>();
    if (lhs_ptr && rhs_ptr) {
      return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() + rhs_ptr->GetValue()));
    } else {
      throw Runtime::Error("bad addition");
    }
    return ObjectHolder::Own(Runtime::Number(lhs_ptr->GetValue() / rhs_ptr->GetValue()));
  }

  ObjectHolder Compound::Execute(Closure& closure) {
    for (auto& statement : statements) {
      auto result_holder = statement->Execute(closure);
      if (result_holder && result_holder.IsReturnable()) {
        return result_holder;
      }
    }

    return ObjectHolder::None();
  }

  ObjectHolder Return::Execute(Closure& closure) { 
    auto statement_oh = statement->Execute(closure);
    statement_oh.MakrReturnable();
    return statement_oh;
  }

  ClassDefinition::ClassDefinition(ObjectHolder class_) : class_(class_) {}

  ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {}

  FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv)
      : object(std::move(object)), field_name(std::move(field_name)), right_value(std::move(rv)) {}

  ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
    // TODO: Check for nullptr
    auto instance_ptr = object.Execute(closure).TryAs<Runtime::ClassInstance>();
    instance_ptr->Fields()[field_name] = right_value->Execute(closure);
    return instance_ptr->Fields()[field_name];
  }

  IfElse::IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
                 std::unique_ptr<Statement> else_body)
      : condition(move(condition)), if_body(move(if_body)), else_body(move(else_body)) {}

  ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
    auto condition_result_oh = condition->Execute(closure);

    if (transform_object_holder_to_bool(condition_result_oh)) {
      return if_body->Execute(closure);
    } else if (else_body) {
      return else_body->Execute(closure);
    }

    return ObjectHolder::None();
  }

  ObjectHolder Or::Execute(Runtime::Closure& closure) {
    auto lhs_oh = lhs->Execute(closure);
    auto rhs_oh = rhs->Execute(closure);
    auto lhs_bool = transform_object_holder_to_bool(lhs_oh);
    auto rhs_bool = transform_object_holder_to_bool(rhs_oh);
    return ObjectHolder::Own(Runtime::Bool(lhs_bool || rhs_bool));
  }

  ObjectHolder And::Execute(Runtime::Closure& closure) {
    auto lhs_oh = lhs->Execute(closure);
    auto rhs_oh = rhs->Execute(closure);
    auto lhs_bool = transform_object_holder_to_bool(lhs_oh);
    auto rhs_bool = transform_object_holder_to_bool(rhs_oh);
    return ObjectHolder::Own(Runtime::Bool(lhs_bool && rhs_bool));
  }

  ObjectHolder Not::Execute(Runtime::Closure& closure) {
    auto arg_oh = argument->Execute(closure);
    auto arg_bool = transform_object_holder_to_bool(arg_oh);
    return ObjectHolder::Own(Runtime::Bool(!arg_bool));
  }

  Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
      : comparator(move(cmp)), left(move(lhs)), right(move(rhs)) {}

  ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
    auto left_oh = left->Execute(closure);
    auto right_oh = right->Execute(closure);
    return ObjectHolder::Own(Runtime::Bool(comparator(left_oh, right_oh)));
  }

  NewInstance::NewInstance(const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args)
      : class_(class_), args(std::move(args)) {}

  NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {}

  ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
    auto instance = Runtime::ClassInstance(class_);
    vector<ObjectHolder> computed_args;
    transform(args.begin(), args.end(), back_inserter(computed_args),
              [&closure](auto& arg) { return arg->Execute(closure); });

    if (instance.HasMethod("__init__", computed_args.size())) {
      instance.Call("__init__", computed_args);
    } else if (!computed_args.empty()) {
      throw Runtime::Error("class " + class_.GetName() + " doesn't have an __init__ method with " +
                           to_string(computed_args.size()) + " argument(s)");
    }
    return ObjectHolder::Own(move(instance));
  }

} /* namespace Ast */

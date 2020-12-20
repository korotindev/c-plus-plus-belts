#include "filters.h"

using namespace std;

CompaniesFilter::CompaniesFilter(const Json::Dict& attrs) {
  if (attrs.count("names")) {
    for (const auto& name : attrs.at("names").AsArray()) {
      names.push_back(name.AsString());
    }
  }
  if (attrs.count("urls")) {
    for (const auto& url : attrs.at("urls").AsArray()) {
      urls.push_back(url.AsString());
    }
  }
  if (attrs.count("rubrics")) {
    for (const auto& rubric : attrs.at("rubrics").AsArray()) {
      rubrics.push_back(rubric.AsString());
    }
  }

  if (attrs.count("phones")) {
    for (const auto& phone_node : attrs.at("phones").AsArray()) {
      const auto& phone_dict = phone_node.AsMap();
      YellowPages::Phone phone;
      if (phone_dict.count("type")) {
        if (phone_dict.at("type").AsString() == "FAX") {
          phone.set_type(YellowPages::Phone_Type::Phone_Type_FAX);
        } else {
          phone.set_type(YellowPages::Phone_Type::Phone_Type_PHONE);
        }
      } else {
        phone.set_type(YellowPages::Phone_Type::Phone_Type_UNKNOWN);
      }

      if (phone_dict.count("country_code")) {
        *phone.mutable_country_code() = phone_dict.at("country_code").AsString();
      }
      if (phone_dict.count("local_code")) {
        *phone.mutable_local_code() = phone_dict.at("local_code").AsString();
      }
      if (phone_dict.count("number")) {
        *phone.mutable_number() = phone_dict.at("number").AsString();
      }
      if (phone_dict.count("extension")) {
        *phone.mutable_extension() = phone_dict.at("extension").AsString();
      }

      phones.push_back(move(phone));
    }
  }
}
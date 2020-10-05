#include <iostream>
#include <map>
#include <string>
#include <vector>

// Serialization

template <typename T>
void Serialize(const T &pod, std::ostream& out);

void Serialize(const std::string& str, std::ostream& out);

template <typename T>
void Serialize(const std::vector<T>& data, std::ostream& out);

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& data, std::ostream& out);


// Deserialization

template <typename T>
void Deserialize(std::istream& in, T& pod);

void Deserialize(std::istream& in, std::string& str);

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& data);

template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& data);


template <typename T>
void Serialize(const T& pod, std::ostream& out) {
  out.write(reinterpret_cast<const char*>(&pod), sizeof(T));
}

void Serialize(const std::string& str, std::ostream& out) {
  auto size = str.size();
  out.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  out.write(str.c_str(), size);
}

template <typename T>
void Serialize(const std::vector<T>& data, std::ostream& out) {
  auto size = data.size();
  out.write(reinterpret_cast<const char*>(&size), sizeof(size));
  for (const T& elem : data) {
    Serialize(elem, out);
  }
}

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& data, std::ostream& out) {
  auto size = data.size();
  out.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  for (const auto& [key, value] : data) {
    Serialize(key, out);
    Serialize(value, out);
  }
}

// Deserialization

template <typename T>
void Deserialize(std::istream& in, T& pod) {
  in.read(reinterpret_cast<char*>(&pod), sizeof(T));
}

void Deserialize(std::istream& in, std::string& str) {
  size_t size = 0;
  in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
  str.resize(size);
  in.read(str.data(), str.size());
}


template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& data) {
  size_t size = 0;
  in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
  while (size--) {
    T1 t1;
    Deserialize(in, t1);
    T2 t2;
    Deserialize(in, t2);
    data[t1] = t2;
  }
}

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& data) {
  size_t size = 0;
  in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
  data.resize(size);
  for (auto& elem : data) {
    Deserialize(in, elem);
  }
}

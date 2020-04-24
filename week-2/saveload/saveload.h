#include <iostream>
#include <map>
#include <string>
#include <vector>

// Serialization

template <typename T>
void Serialize(T pod, std::ostream& out);

void Serialize(const std::string& str, std::ostream& out);

template <typename T>
void Serialize(const std::vector<T>& data, std::ostream& out);

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& data, std::ostream& out);

template <typename T>
void Serialize(T pod, std::ostream& out) {
  out.write(reinterpret_cast<const char*>(&pod), sizeof(pod));
}

void Serialize(const std::string& str, std::ostream& out) {
  Serialize(str.size(), out);
  out.write(str.c_str(), str.size());
}

template <typename T>
void Serialize(const std::vector<T>& data, std::ostream& out) {
  Serialize(data.size(), out);
  for (const auto& e : data) {
    Serialize(e, out);
  }
}

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& data, std::ostream& out) {
  Serialize(data.size(), out);
  for (const auto& [k, v] : data) {
    Serialize(k, out);
    Serialize(v, out);
  }
}

// Deserialization

template <typename T>
void Deserialize(std::istream& in, T& pod);

void Deserialize(std::istream& in, std::string& str);

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& data);

template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& data);


template <typename T>
void Deserialize(std::istream& in, T& pod) {
  in.read(reinterpret_cast<char*>(&pod), sizeof(pod));
}

void Deserialize(std::istream& in, std::string& str) {
  auto sz = str.size();
  Deserialize(in, sz);
  str.resize(sz);
  in.read(str.data(), sz);
}

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& data) {
  auto sz = data.size();
  Deserialize(in, sz);
  data.resize(sz);
  for (auto& e : data) {
    Deserialize(in, e);
  }
}

template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& data) {
  auto sz = data.size();
  Deserialize(in, sz);
  for (int i = 0; i < sz; ++i) {
    T1 lhs;
    T2 rhs;
    Deserialize(in, lhs);
    Deserialize(in, rhs);
    data[lhs] = rhs;
  }
}

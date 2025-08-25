#ifndef JSON_HPP
#define JSON_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#ifdef USE_QLS_STRING_PARAM
#include <string_param.hpp>
#else
#include <cassert>
#include <memory_resource>
#include <stdexcept>
#include <variant>
#endif // USE_QLS_STRING_PARAM

namespace qjson {
/**
 * @brief Enumeration for JSON value types.
 */
enum JValueType : std::uint8_t {
  JNull,
  JInt,
  JDouble,
  JBool,
  JString,
  JList,
  JDict
};

#ifndef USE_QLS_STRING_PARAM

class string_param {
public:
  string_param(std::string_view view) : is_owned_(false), view_(view) {}

  string_param(const std::string &str) : is_owned_(false), view_(str) {}

  string_param(std::string &&str) : is_owned_(true), buffer_(std::move(str)) {}

  string_param(const std::pmr::string &str) : is_owned_(false), view_(str) {}

  string_param(std::pmr::string &&str)
      : is_owned_(true), buffer_(std::move(str)) {}

  template <typename It, std::sentinel_for<It> S>
  string_param(It first, S last) : is_owned_(false), view_(first, last) {}

  template <std::size_t N>
  string_param(const char (&str)[N], std::size_t size = N - 1)
      : is_owned_(false), view_(str, size) {
    assert(size < N);
  }

  string_param(const char *str) {
    if (str) {
      is_owned_ = false;
      view_ = std::string_view(str);
    } else {
      throw std::invalid_argument("Null pointer passed to string_param");
    }
  }

  string_param(const string_param &) = delete;
  string_param &operator=(const string_param &) = delete;

  string_param(string_param &&str)
      : is_owned_(str.is_owned_), view_(str.view_),
        buffer_(std::move(str.buffer_)) {}

  string_param &operator=(string_param &&str) {
    if (this != &str) {
      is_owned_ = str.is_owned_;
      view_ = str.view_;
      buffer_ = std::move(str.buffer_);
    }
    return *this;
  }

  std::size_t size() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return s.size(); }, buffer_);
    }
    return view_.size();
  }

  operator std::string_view() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return std::string_view(s); },
                        buffer_);
    }
    return view_;
  }

  bool is_owned() const { return is_owned_; }

  bool is_std() const {
    if (is_owned_) {
      return std::holds_alternative<std::string>(buffer_);
    }
    return false;
  }

  bool is_pmr() const {
    if (is_owned_) {
      return std::holds_alternative<std::pmr::string>(buffer_);
    }
    return false;
  }

  template <typename T,
            typename = std::enable_if_t<std::is_same_v<T, std::string> ||
                                        std::is_same_v<T, std::pmr::string>>>
  T extract() && {
    if (is_owned_) {
      return std::get<T>(std::move(buffer_));
    }
    throw std::logic_error("Cannot extract from non-owned string_param");
  }

  std::string extract_std() && {
    return std::move(*this).extract<std::string>();
  }

  std::pmr::string extract_pmr() && {
    return std::move(*this).extract<std::pmr::string>();
  }

  friend bool operator==(const string_param &lhs, const string_param &rhs) {
    return std::string_view(lhs) == std::string_view(rhs);
  }

  friend bool operator<(const string_param &lhs, const string_param &rhs) {
    return std::string_view(lhs) < std::string_view(rhs);
  }

  friend bool operator!=(const string_param &lhs, const string_param &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<=(const string_param &lhs, const string_param &rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const string_param &lhs, const string_param &rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const string_param &lhs, const string_param &rhs) {
    return !(lhs < rhs);
  }

private:
  std::string_view view_;
  std::variant<std::string, std::pmr::string> buffer_;
  bool is_owned_;
};

#else
using string_param = qls::string_param;
#endif // STRING_PARAM_HPP

struct string_hash {
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  std::size_t operator()(const char *str) const { return hash_type{}(str); }
  std::size_t operator()(std::string_view str) const {
    return hash_type{}(str);
  }
  std::size_t operator()(const std::string &str) const {
    return hash_type{}(str);
  }
  std::size_t operator()(const std::pmr::string &str) const {
    return hash_type{}(str);
  }
  std::size_t operator()(const string_param &str) const {
    return hash_type{}(str);
  }
};

class JObject;

using null_t = bool;
using int_t = long long;
using bool_t = bool;
using double_t = long double;
using string_t = std::pmr::string;
using list_t = std::pmr::vector<JObject>;
using dict_t =
    std::pmr::unordered_map<string_t, JObject, string_hash, std::equal_to<>>;
using value_t = std::variant<int_t, bool_t, double_t, string_t, list_t, dict_t>;

/**
 * @brief Class representing a JSON object.
 */
class JObject {
public:
  JObject();
  JObject(const JObject &jobject);
  JObject(JObject &&jobject) noexcept;
  JObject(JValueType jvt);
  JObject(int_t value);
  JObject(long value);
  JObject(int value);
  JObject(short value);
  JObject(bool_t value);
  JObject(double_t value);
  JObject(double value);
  JObject(float value);
  JObject(const std::string &data);
  JObject(std::string_view data);
  JObject(const string_param &data);
  JObject(string_param &&data);
  JObject(std::string &&data) noexcept;
  JObject(const string_t &data);
  JObject(string_t &&data) noexcept;
  ~JObject();

  JObject &operator=(const JObject &jobject);
  JObject &operator=(JObject &&jobject) noexcept;

  friend bool operator==(const JObject &joa, const JObject &jobject);
  friend bool operator==(const JObject &jobject, JValueType type);

  const JObject &operator[](std::size_t iter) const;
  JObject &operator[](std::size_t iter);
  const JObject &operator[](std::string_view str) const;
  JObject &operator[](std::string_view str);

  void push_back(const JObject &jobject);
  void push_back(JObject &&jobject);
  void pop_back();
  bool hasMember(const std::string &str) const;

  JValueType getType() const noexcept;
  const list_t &getList() const;
  list_t &getList();
  const dict_t &getDict() const;
  dict_t &getDict();
  const int_t &getInt() const;
  int_t &getInt();
  const double_t &getDouble() const;
  double_t &getDouble();
  const bool_t &getBool() const;
  bool_t &getBool();
  std::string getString() const;
  string_t &getPMRString();
  const string_t &getPMRString() const;

  std::string to_string() const;
  std::string to_string(std::size_t indent) const;
  static JObject to_json(string_param data);

private:
  value_t m_value;   ///< The value of the JSON object.
  JValueType m_type; ///< The type of the JSON value.
};

JObject operator""_qjson(const char *data, std::size_t length);
JObject to_json(string_param data);
std::string to_string(const JObject &jobject);
std::string to_string(const JObject &jobject, std::size_t indent);

/**
 * @brief Class for parsing JSON data.
 */
class JParser {
public:
  JParser() = default;

  /**
   * @brief Parses JSON data from a string view.
   * @param data The JSON data to parse.
   * @return The parsed JSON object.
   */
  JObject parse(string_param data);

protected:
  JObject parse_(std::string_view data, std::size_t data_size,
                 std::size_t &iter);
  static void skipSpace(std::string_view data, std::size_t data_size,
                        std::size_t &iter, long long &error_line);
  static std::pmr::string getString(std::string_view data,
                                    std::size_t data_size, std::size_t &iter,
                                    long long error_line);
  static JObject getNumber(std::string_view data, std::size_t data_size,
                           std::size_t &iter, long long error_line);
  static JObject getBool(std::string_view data, std::size_t data_size,
                         std::size_t &iter, long long error_line);
  static JObject getNull(std::string_view data, std::size_t data_size,
                         std::size_t &iter, long long error_line);
  static std::string getLogicErrorString(long long error_line);
  static std::string getLogicErrorString(long long error_line,
                                         std::string_view error);
};

/**
 * @brief Class for writing JSON data.
 */
class JWriter {
public:
  JWriter() = default;
  ~JWriter() = default;

  /**
   * @brief Writes a JSON object to a string.
   * @param jobject The JSON object to write.
   * @return The JSON data as a string.
   */
  std::string write(const JObject &jobject);

  /**
   * @brief Writes a formatted JSON object to a string.
   * @param jobject The JSON object to write.
   * @param n The indentation level.
   * @return The formatted JSON data as a string.
   */
  std::string formatWrite(const JObject &jobject, std::size_t indent = 4,
                          std::size_t n = 1);

private:
  std::size_t getJObjectSurmisedSize(const JObject &jobject);
  void write_(const JObject &jobject, std::string &buffer);
};
} // namespace qjson

#ifndef USE_QLS_STRING_PARAM
namespace std {
template <> struct formatter<qjson::string_param, char> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const qjson::string_param &str_param, FormatContext &ctx) const {
    return std::ranges::copy(std::string_view(str_param), ctx.out()).out;
  }
};
} // namespace std
#endif // USE_QLS_STRING_PARAM

#endif // !JSON_HPP

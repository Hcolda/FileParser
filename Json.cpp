#include "Json.h"

#include <cmath>
#include <cstring>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#define JSON_NAMESPACE_START namespace qjson {
#define JSON_NAMESPACE_END }

JSON_NAMESPACE_START

JObject::JObject() : m_type(JValueType::JNull), m_value(null_t()) {}

JObject::JObject(const JObject &jobject)
    : m_type(jobject.m_type), m_value(jobject.m_value) {}

JObject::JObject(JObject &&jobject) noexcept
    : m_type(jobject.m_type), m_value(std::move(jobject.m_value)) {
  jobject.m_type = JNull;
}

JObject::JObject(JValueType jvt) : m_type(jvt) {
  switch (jvt) {
  case qjson::JValueType::JNull:
    m_value = null_t();
    break;
  case qjson::JValueType::JInt:
    m_value = int_t();
    break;
  case qjson::JValueType::JDouble:
    m_value = double_t();
    break;
  case qjson::JValueType::JBool:
    m_value = bool_t();
    break;
  case qjson::JValueType::JString:
    m_value = string_t(std::pmr::get_default_resource());
    break;
  case qjson::JValueType::JList:
    m_value = list_t(std::pmr::get_default_resource());
    break;
  case qjson::JValueType::JDict:
    m_value = dict_t(std::pmr::get_default_resource());
    break;
  default:
    break;
  }
}

JObject::JObject(int_t value) : m_type(JValueType::JInt), m_value(value) {}

JObject::JObject(long value)
    : m_type(JValueType::JInt), m_value(static_cast<int_t>(value)) {}

JObject::JObject(int value)
    : m_type(JValueType::JInt), m_value(static_cast<int_t>(value)) {}

JObject::JObject(short value)
    : m_type(JValueType::JInt), m_value(static_cast<int_t>(value)) {}

JObject::JObject(bool_t value) : m_type(JValueType::JBool), m_value(value) {}

JObject::JObject(double_t value)
    : m_type(JValueType::JDouble), m_value(value) {}

JObject::JObject(double value)
    : m_type(JValueType::JDouble), m_value(static_cast<double_t>(value)) {}

JObject::JObject(float value)
    : m_type(JValueType::JDouble), m_value(static_cast<double_t>(value)) {}

JObject::JObject(const std::string &data)
    : m_type(JValueType::JString),
      m_value(string_t(data, std::pmr::get_default_resource())) {}

qjson::JObject::JObject(std::string_view data)
    : m_type(JValueType::JString),
      m_value(string_t(data, std::pmr::get_default_resource())) {}

JObject::JObject(std::string &&data) noexcept
    : m_type(JValueType::JString),
      m_value(string_t(data, std::pmr::get_default_resource())) {}

JObject::JObject(const string_t &data)
    : m_type(JValueType::JString),
      m_value(string_t(data, std::pmr::get_default_resource())) {}

JObject::JObject(string_t &&data) noexcept
    : m_type(JValueType::JString), m_value(std::move(data)) {}

JObject::~JObject() = default;

JObject &JObject::operator=(const JObject &jobject) {
  if (this == &jobject) {
    return *this;
  }

  m_type = jobject.m_type;
  m_value = jobject.m_value;
  return *this;
}

JObject &JObject::operator=(JObject &&jobject) noexcept {
  if (this == &jobject) {
    return *this;
  }

  m_type = std::exchange(jobject.m_type, JNull);
  m_value = std::move(jobject.m_value);
  return *this;
}

bool operator==(const JObject &joa, const JObject &jobject) {
  if (joa.m_type != jobject.m_type) {
    return false;
  }
  switch (jobject.m_type) {
  case JValueType::JNull:
    return true;
  case JValueType::JInt:
    return joa.getInt() == jobject.getInt();
  case JValueType::JDouble:
    return joa.getDouble() == jobject.getDouble();
  case JValueType::JBool:
    return joa.getBool() == jobject.getBool();
  case JValueType::JString:
    return joa.getString() == jobject.getString();
  case JValueType::JList: {
    const list_t &local = joa.getList();
    const list_t &jolist = jobject.getList();
    if (local.empty() ^ jolist.empty()) {
      return false;
    }
    if (local.size() != jolist.size()) {
      return false;
    }
    for (std::size_t i = 0; i < local.size(); i++) {
      if (!(local[i] == jolist[i])) {
        return false;
      }
    }
    return true;
  }
  case JValueType::JDict: {
    const dict_t &local = joa.getDict();
    const dict_t &joDict = jobject.getDict();
    if (local.empty() && joDict.empty()) {
      return true;
    }
    if (local.empty() != joDict.empty()) {
      return false;
    }
    if (local.size() != joDict.size()) {
      return false;
    }
    for (auto i = local.begin(); i != local.end(); ++i) {
      if (joDict.find(i->first) == joDict.end()) {
        return false;
      }
      if (!(i->second == joDict.find(i->first)->second)) {
        return false;
      }
    }
    return true;
  }
  default:
    return false;
  }
}

bool operator==(const JObject &jobject, JValueType type) {
  return jobject.m_type == type;
}

const JObject &JObject::operator[](std::size_t iter) const {
  if (m_type != JValueType::JNull && m_type != JValueType::JList) {
    throw std::logic_error("The type isn't JList.");
  }
  if (m_type == JValueType::JNull) {
    throw std::logic_error("The type is JNull.");
  }
  const auto *const local_list = std::get_if<list_t>(&m_value);
  if (iter >= local_list->size()) {
    throw std::logic_error("The size is smaller than iter.");
  }
  return (*local_list).at(iter);
}

JObject &JObject::operator[](std::size_t iter) {
  if (m_type != JValueType::JNull && m_type != JValueType::JList) {
    throw std::logic_error("The type isn't JList.");
  }
  if (m_type == JValueType::JNull) {
    m_type = JValueType::JList;
    m_value = list_t(std::pmr::get_default_resource());
  }
  auto *local_list = std::get_if<list_t>(&m_value);
  if (iter >= local_list->size()) {
    local_list->resize(iter + 1);
  }
  return local_list->at(iter);
}

const JObject &JObject::operator[](std::string_view str) const {
  if (m_type != JValueType::JNull && m_type != JValueType::JDict) {
    throw std::logic_error("The type isn't JDict.");
  }
  if (m_type == JValueType::JNull) {
    throw std::logic_error("The type is JNull.");
  }
  const auto *const local_dict = std::get_if<dict_t>(&m_value);
  auto iter = local_dict->find(str);
  if (iter == local_dict->cend()) {
    throw std::logic_error("Could not find the element.");
  }
  return iter->second;
}

JObject &JObject::operator[](std::string_view str) {
  if (m_type != JValueType::JNull && m_type != JValueType::JDict) {
    throw std::logic_error("The type isn't JDict.");
  }
  if (m_type == JValueType::JNull) {
    m_type = JValueType::JDict;
    m_value = dict_t(std::pmr::get_default_resource());
  }
  auto *local_dict = std::get_if<dict_t>(&m_value);
  auto iter = local_dict->find(str);
  if (iter == local_dict->cend()) {
    return local_dict->emplace(str, JObject{}).first->second;
  }
  return iter->second;
}

void JObject::push_back(const JObject &jobject) {
  if (m_type != JValueType::JNull && m_type != JValueType::JList) {
    throw std::logic_error("The type isn't JList.");
  }
  if (m_type == JValueType::JNull) {
    m_type = JValueType::JList;
    m_value = list_t(std::pmr::get_default_resource());
    std::get_if<list_t>(&m_value)->push_back(jobject);
  }
  std::get_if<list_t>(&m_value)->push_back(jobject);
}

void JObject::push_back(JObject &&jobject) {
  if (m_type != JValueType::JNull && m_type != JValueType::JList) {
    throw std::logic_error("The type isn't JList.");
  }
  if (m_type == JValueType::JNull) {
    m_type = JValueType::JList;
    m_value = list_t(std::pmr::get_default_resource());
    std::get_if<list_t>(&m_value)->push_back(std::move(jobject));
  }
  std::get_if<list_t>(&m_value)->push_back(std::move(jobject));
}

void JObject::pop_back() {
  if (m_type == JValueType::JList) {
    list_t *local = std::get_if<list_t>(&m_value);
    if (local->empty()) {
      throw std::logic_error("The JList is empty.");
    }
    local->pop_back();
    return;
  }
  throw std::logic_error("The type isn't JList.");
}

bool JObject::hasMember(const std::string &str) const {
  if (m_type != JValueType::JDict) {
    throw std::logic_error("The type isn't JDict.");
  }
  const dict_t *local = std::get_if<dict_t>(&m_value);
  return local->find(std::string_view(str)) != local->cend();
}

JValueType JObject::getType() const noexcept { return m_type; }

const list_t &JObject::getList() const {
  if (m_type != JValueType::JList) {
    throw std::logic_error("The type isn't JList.");
  }
  return *std::get_if<list_t>(&m_value);
}

list_t &JObject::getList() {
  if (m_type != JValueType::JList) {
    throw std::logic_error("The type isn't JList.");
  }
  return *std::get_if<list_t>(&m_value);
}

const dict_t &JObject::getDict() const {
  if (m_type != JValueType::JDict) {
    throw std::logic_error("The type isn't JDict.");
  }
  return *std::get_if<dict_t>(&m_value);
}

dict_t &JObject::getDict() {
  if (m_type != JValueType::JDict) {
    throw std::logic_error("The type isn't JDict.");
  }
  return *std::get_if<dict_t>(&m_value);
}

const int_t &JObject::getInt() const {
  if (m_type != JValueType::JInt) {
    throw std::logic_error("This JObject isn't int");
  }
  return *std::get_if<int_t>(&m_value);
}

int_t &JObject::getInt() {
  if (m_type != JValueType::JInt) {
    throw std::logic_error("This JObject isn't int");
  }
  return *std::get_if<int_t>(&m_value);
}

const double_t &JObject::getDouble() const {
  if (m_type != JValueType::JDouble) {
    throw std::logic_error("This JObject isn't double");
  }
  return *std::get_if<double_t>(&m_value);
}

double_t &JObject::getDouble() {
  if (m_type != JValueType::JDouble) {
    throw std::logic_error("This JObject isn't double");
  }
  return *std::get_if<double_t>(&m_value);
}

const bool_t &JObject::getBool() const {
  if (m_type != JValueType::JBool) {
    throw std::logic_error("This JObject isn't bool");
  }
  return *std::get_if<bool_t>(&m_value);
}

bool_t &JObject::getBool() {
  if (m_type != JValueType::JBool) {
    throw std::logic_error("This JObject isn't bool");
  }
  return *std::get_if<bool_t>(&m_value);
}

std::string JObject::getString() const {
  if (m_type != JValueType::JString) {
    throw std::logic_error("This JObject isn't string");
  }
  const auto *const ptr = std::get_if<string_t>(&m_value);
  return {ptr->begin(), ptr->end()};
}

std::pmr::string &JObject::getPMRString() {
  if (m_type != JValueType::JString) {
    throw std::logic_error("This JObject isn't string");
  }
  return *std::get_if<string_t>(&m_value);
}

const std::pmr::string &JObject::getPMRString() const {
  if (m_type != JValueType::JString) {
    throw std::logic_error("This JObject isn't string");
  }
  return *std::get_if<string_t>(&m_value);
}

std::string JObject::to_string() const {
  JWriter jwriter;
  return jwriter.write(*this);
}

std::string JObject::to_string(std::size_t indent) const {
  JWriter jwriter;
  if (!indent) {
    return jwriter.write(*this);
  }
  return jwriter.formatWrite(*this, indent);
}

JObject qjson::JObject::to_json(std::string_view data) {
  JParser jparser;
  return jparser.parse(data);
}

JObject operator""_qjson(const char *data, std::size_t length) {
  JParser jparser;
  return jparser.parse(std::string_view{data, length});
}

JObject to_json(std::string_view data) { return JObject::to_json(data); }

std::string to_string(const JObject &jobject) {
  JWriter jwriter;
  return jwriter.write(jobject);
}

std::string to_string(const JObject &jobject, std::size_t indent) {
  JWriter jwriter;
  if (!indent) {
    return jwriter.write(jobject);
  }
  return jwriter.formatWrite(jobject, indent);
}

JObject JParser::parse(std::string_view data) {
  std::size_t iter = 0;
  return parse_(data, data.size(), iter);
}

JObject JParser::parse_(std::string_view data, std::size_t data_size,
                        std::size_t &iter) {
  long long error_line = 0;

  if (data.empty()) {
    throw std::logic_error(getLogicErrorString(error_line));
  }
  skipSpace(data, data_size, iter, error_line);
  if (data_size <= iter) {
    throw std::logic_error(getLogicErrorString(error_line));
  }

  if (data[iter] == '{') {
    JObject localJO(JValueType::JDict);
    ++iter;
    while (iter < data_size && data[iter] != '}') {
      skipSpace(data, data_size, iter, error_line);
      if (data[iter] == '}') {
        return localJO;
      }
      std::pmr::string key(getString(data, data_size, iter, error_line));
      skipSpace(data, data_size, iter, error_line);
      if (data[iter] == ':') {
        ++iter;
      } else {
        throw std::logic_error(getLogicErrorString(error_line));
      }
      skipSpace(data, data_size, iter, error_line);
      localJO[key] = parse_(data, data_size, iter);
      skipSpace(data, data_size, iter, error_line);
      if (data[iter] != ',' && data[iter] != '}') {
        throw std::logic_error(getLogicErrorString(error_line));
      }
      if (data[iter] == '}') {
        ++iter;
        return localJO;
      }
      ++iter;
      skipSpace(data, data_size, iter, error_line);
    }
    if (data[iter] == '}') {
      ++iter;
      return localJO;
    }
    throw std::logic_error(getLogicErrorString(error_line));
  }
  if (data[iter] == '[') {
    JObject localJO(JValueType::JList);
    ++iter;
    while (iter < data_size && data[iter] != ']') {
      skipSpace(data, data_size, iter, error_line);
      if (data[iter] == ']') {
        return localJO;
      }
      localJO.push_back(parse_(data, data_size, iter));
      skipSpace(data, data_size, iter, error_line);
      if (data[iter] != ',' && data[iter] != ']') {
        throw std::logic_error(getLogicErrorString(error_line));
      }
      if (data[iter] == ']') {
        ++iter;
        return localJO;
      }
      ++iter;
      skipSpace(data, data_size, iter, error_line);
    }
    if (data[iter] == ']') {
      return localJO;
    }

    throw std::logic_error(getLogicErrorString(error_line));
  }
  if (data[iter] == '\"') {
    return getString(data, data_size, iter, error_line);
  }
  if (data[iter] == 'n') {
    return getNull(data, data_size, iter, error_line);
  }
  if (data[iter] == 't' || data[iter] == 'f') {
    return getBool(data, data_size, iter, error_line);
  }
  if ((data[iter] >= '0' && data[iter] <= '9') || data[iter] == '-') {
    return getNumber(data, data_size, iter, error_line);
  }
  throw std::logic_error(getLogicErrorString(error_line));
}

void JParser::skipSpace(std::string_view data, std::size_t data_size,
                        std::size_t &iter, long long &error_line) {
  while (iter < data_size &&
         (data[iter] == ' ' || data[iter] == '\t' || data[iter] == '\n')) {
    if (data[iter] == '\n') {
      ++error_line;
    }
    ++iter;
  }
}

string_t JParser::getString(std::string_view data, std::size_t data_size,
                            std::size_t &iter, long long error_line) {
  if (data[iter] == '\"') {
    string_t str{std::pmr::get_default_resource()};
    ++iter;
    while (iter < data_size && data[iter] != '\"') {
      if (data[iter] == '\\') {
        ++iter;
        switch (data[iter]) {
        case 'n':
          str += '\n';
          break;
        case 'b':
          str += "\b";
          break;
        case 'f':
          str += "\f";
          break;
        case 'r':
          str += "\r";
          break;
        case 't':
          str += "\t";
          break;
        case '\\':
          str += "\\";
          break;
        case '\"':
          str += "\"";
          break;
        case '/':
          str += "/";
          break;
        default:
          throw std::logic_error(getLogicErrorString(error_line));
          break;
        }
      } else {
        str += data[iter];
      }
      ++iter;
    }
    if (iter >= data_size) {
      throw std::logic_error(getLogicErrorString(error_line));
    }
    ++iter;
    return str;
  }
  throw std::logic_error(getLogicErrorString(error_line));
}

JObject JParser::getNumber(std::string_view data, std::size_t data_size,
                           std::size_t &iter, long long error_line) {
  bool isDouble = false;
  bool firstNum = false;
  bool isNegative = false;
  if (data[iter] == '-') {
    isNegative = true;
    ++iter;
  }
  std::size_t count = 0;
  std::size_t start = iter;

  while (iter < data_size &&
         ((data[iter] >= '0' && data[iter] <= '9') || data[iter] == '.')) {
    if (!firstNum && data[iter] >= '0' && data[iter] <= '9') {
      firstNum = true;
    } else if (isDouble) {
      count++;
    } else if (data[iter] == '.') {
      if (!firstNum) {
        throw std::logic_error(getLogicErrorString(error_line));
      }
      isDouble = true;
      ++iter;
      continue;
    }
    ++iter;
  }

  if (isDouble) {
    double_t number = data[iter - 1] - '0';
    std::size_t single = 10;
    for (long long i = iter - 2; i >= static_cast<long long>(start);
         --i, single *= 10) {
      if (data[i] == '.') {
        continue;
      }
      number += single * (data[i] - '0');
    }
    if (isNegative) {
      number *= -1;
    }
    return number / std::pow(10, count);
  }
  long long number = data[iter - 1] - '0';
  std::size_t single = 10;
  for (long long i = iter - 2; i >= static_cast<long long>(start);
       --i, single *= 10) {
    number += single * (data[i] - '0');
  }
  if (isNegative) {
    number *= -1;
  }
  return number;
}

JObject JParser::getBool(std::string_view data, std::size_t data_size,
                         std::size_t &iter, long long error_line) {
  constexpr std::size_t true_size = 4;
  constexpr std::size_t false_size = 5;
  if (data_size >= iter + true_size &&
      static_cast<bool>(std::memcmp(data.data(), "true", true_size))) {
    iter += 4;
    return true;
  }
  if (data_size >= iter + false_size &&
      static_cast<bool>(std::memcmp(data.data(), "false", false_size))) {
    iter += false_size;
    return false;
  }
  throw std::logic_error(getLogicErrorString(error_line));
}

JObject JParser::getNull(std::string_view data, std::size_t data_size,
                         std::size_t &iter, long long error_line) {
  constexpr std::size_t null_size = 4;
  if (data_size >= iter + null_size &&
      static_cast<bool>(std::memcmp(data.data(), "null", null_size))) {
    iter += null_size;
    return JObject();
  }
  throw std::logic_error(getLogicErrorString(error_line));
}

std::string JParser::getLogicErrorString(long long error_line) {
  return "Invalid Input, in line " + std::to_string(error_line);
}

std::string qjson::JParser::getLogicErrorString(long long error_line,
                                                std::string_view error) {
  return std::string(error) + " , in line " + std::to_string(error_line);
}

std::size_t JWriter::getJObjectSurmisedSize(const JObject &jobject) {
  std::size_t count = 0;
  switch (jobject.getType()) {
  case JValueType::JNull:
    count += 4; // "null"
    break;
  case JValueType::JInt:
    count += sizeof(int_t) * 2; // "1234567890"
    break;
  case JValueType::JDouble:
    count += sizeof(double_t) * 2; // "1234567890.1234567890"
    break;
  case JValueType::JBool:
    count += 5; // "true" or "false"
    break;
  case JValueType::JString: {
    const std::pmr::string &localString = jobject.getPMRString();
    if (localString.empty()) {
      count += 2; // ""
    } else {
      count += localString.size() + 2; // "string"
    }
    break;
  }
  case JValueType::JList: {
    const list_t &list = jobject.getList();
    if (list.empty()) {
      count += 2; // []
    } else {
      for (const auto &item : list) {
        count += getJObjectSurmisedSize(item) + 1; // item,
      }
      count += list.size() + 2; // [item1,item2,...]
    }
    break;
  }
  case JValueType::JDict: {
    const dict_t &dict = jobject.getDict();
    if (dict.empty()) {
      count += 2; // {}
    } else {
      for (const auto &[key, value] : dict) {
        count += key.size() + getJObjectSurmisedSize(value) + 4; // "key":value,
      }
      count += dict.size() + 2; // {key1:value1,key2:value2,...}
    }
    break;
  }
  default:
    break;
  }
  return count;
}

void JWriter::write_(const JObject &jobject, std::string &buffer) {
  std::string &str = buffer;
  str.reserve(
      getJObjectSurmisedSize(jobject)); // Reserve space to avoid reallocations
  switch (jobject.getType()) {
  case JValueType::JNull:
    str += "null";
    break;
  case JValueType::JInt:
    str += std::to_string(jobject.getInt());
    break;
  case JValueType::JDouble:
    str += std::to_string(jobject.getDouble());
    break;
  case JValueType::JBool:
    if (jobject.getBool()) {
      str += "true";
    } else {
      str += "false";
    }
    break;
  case JValueType::JString: {
    const std::pmr::string &localString = jobject.getPMRString();
    if (localString.empty()) {
      str += "\"\"";
    } else {
      str += '\"';
      for (const char &iter : localString) {
        switch (iter) {
        case 0:
          throw std::logic_error("Invalid string");
        case '\n':
          str += "\\n";
          break;
        case '\b':
          str += "\\b";
          break;
        case '\f':
          str += "\\f";
          break;
        case '\r':
          str += "\\r";
          break;
        case '\t':
          str += "\\t";
          break;
        case '\\':
          str += "\\\\";
          break;
        case '\"':
          str += "\\\"";
          break;
        default:
          str += iter;
          break;
        }
      }
      str += '\"';
    }
    break;
  }
  case JValueType::JList: {
    const list_t &list = jobject.getList();
    if (list.empty()) {
      str += "[]";
    } else {
      str += '[';
      for (auto iter = list.begin(); iter != list.end(); ++iter) {
        write_(*iter, str);
        if (iter + 1 != list.end()) {
          str += ',';
        }
      }
      str += ']';
    }
    break;
  }
  case JValueType::JDict: {
    const dict_t &dict = jobject.getDict();
    if (dict.empty()) {
      str += "{}";
    } else {
      str += '{';
      for (auto iter = dict.begin(), iter2 = dict.begin(); iter != dict.end();
           ++iter) {
        str += '\"';
        str += iter->first;
        str += "\":";
        write_(iter->second, str);
        iter2 = iter;
        if (++iter2 != dict.end()) {
          str += ',';
        }
      }
      str += '}';
    }
    break;
  }
  default:
    break;
  }
}

std::string JWriter::write(const JObject &jobject) {
  std::string str;
  write_(jobject, str);
  return str;
}

std::string JWriter::formatWrite(const JObject &jobject, size_t indent,
                                 std::size_t n) {
  std::string str;
  std::string indent_space;
  indent_space.resize(indent);
  std::memset(indent_space.data(), ' ', indent);

  switch (jobject.getType()) {
  case JValueType::JNull:
    str += "null";
    break;
  case JValueType::JInt:
    str += std::to_string(jobject.getInt());
    break;
  case JValueType::JDouble:
    str += std::to_string(jobject.getDouble());
    break;
  case JValueType::JBool:
    if (jobject.getBool()) {
      str += "true";
      break;
    }
    str += "false";
    break;
  case JValueType::JString: {
    std::string localString(jobject.getString());
    str += '\"';
    for (const char &iter : localString) {
      switch (iter) {
      case 0:
        throw std::logic_error("Invalid string");
      case '\n':
        str += "\\n";
        break;
      case '\b':
        str += "\\b";
        break;
      case '\f':
        str += "\\f";
        break;
      case '\r':
        str += "\\r";
        break;
      case '\t':
        str += "\\t";
        break;
      case '\\':
        str += "\\\\";
        break;
      case '\"':
        str += "\\\"";
        break;
      default:
        str += iter;
        break;
      }
    }
    str += '\"';
    break;
  }
  case JValueType::JList: {
    const list_t &list = jobject.getList();
    str += "[\n";
    for (auto iter = list.begin(); iter != list.end(); ++iter) {
      for (std::size_t i = 0; i < n; i++) {
        str += indent_space;
      }
      str += formatWrite(*iter, indent, n + 1);
      if (iter + 1 != list.end()) {
        str += ",\n";
      }
    }
    str += '\n';
    for (std::size_t i = 0; i < n - 1; i++) {
      str += indent_space;
    }
    str += ']';
    break;
  }
  case JValueType::JDict: {
    const dict_t &dict = jobject.getDict();
    str += "{\n";
    for (auto iter = dict.begin(), iter2 = dict.begin(); iter != dict.end();
         ++iter) {
      for (std::size_t i = 0; i < n; i++) {
        str += indent_space;
      }
      str += '\"' + std::string(iter->first) +
             "\": " + formatWrite(iter->second, indent, n + 1);
      iter2 = iter;
      if (++iter2 != dict.end()) {
        str += ",\n";
      }
    }
    str += '\n';
    for (std::size_t i = 0; i < n - 1; i++) {
      str += indent_space;
    }
    str += '}';
    break;
  }
  default:
    break;
  }

  return str;
}

JSON_NAMESPACE_END

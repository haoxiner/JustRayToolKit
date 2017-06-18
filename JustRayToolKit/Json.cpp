#include "Json.h"
#include <cctype>
#include <iostream>
namespace JustRay
{
const Json::JsonValue Json::JsonValue::NULL_OBJECT;

Json::JsonValue::JsonValue() : type_(JSON_NULL), data_({ false })
{
}

Json::JsonValue::JsonValue(float number) : type_(JSON_NUMBER), data_({ number })
{
}

Json::JsonValue::JsonValue(bool boolean) : type_(JSON_BOOLEAN), data_({ boolean })
{
}

Json::JsonValue::JsonValue(char* str) : type_(JSON_STRING), data_({ str })
{
}

Json::JsonValue::JsonValue(JsonObject* object) : type_(JSON_OBJECT), data_({ object })
{
}

void Json::JsonValue::Destroy()
{
    if (type_ == JSON_STRING) {
        delete[] data_.str;
        std::cerr << "De-Alloc" << std::endl;
    } else if (type_ == JSON_OBJECT) {
        delete data_.object;
        std::cerr << "De-Alloc" << std::endl;
    } else if (type_ >= JSON_ARRAY) {
        for (auto&& value : *(data_.array)) {
            value.Destroy();
        }
        delete data_.array;
        std::cerr << "De-Alloc" << std::endl;
    }
}

int Json::JsonValue::AsInt() const
{
    if (type_ == JSON_NUMBER) {
        return static_cast<int>(data_.number);
    }
    return 0;
}

bool Json::JsonValue::AsBool() const
{
    return data_.boolean;
}

float Json::JsonValue::AsFloat() const
{
    if (type_ == JSON_NUMBER) {
        return data_.number;
    }
    return 0.0f;
}

Float2 Json::JsonValue::AsFloat2() const
{
    if (type_ == JSON_ARRAY + 2) {
        return Float2((*(data_.array))[0].AsFloat(), (*(data_.array))[1].AsFloat());
    }
    return Float2();
}

Float3 Json::JsonValue::AsFloat3() const
{
    if (type_ == JSON_ARRAY + 3) {
        return Float3((*(data_.array))[0].AsFloat(), (*(data_.array))[1].AsFloat(), (*(data_.array))[2].AsFloat());
    }
    return Float3();
}

Float4 Json::JsonValue::AsFloat4() const
{
    if (type_ == JSON_ARRAY + 4) {
        return Float4((*(data_.array))[0].AsFloat(), (*(data_.array))[1].AsFloat(), (*(data_.array))[2].AsFloat(), (*(data_.array))[3].AsFloat());
    }
    return Float4();
}

Matrix4x4 Json::JsonValue::AsMatrix4x4() const
{
    if (type_ == JSON_ARRAY + 16) {
        return Matrix4x4(
            (*(data_.array))[0].AsFloat(), (*(data_.array))[1].AsFloat(), (*(data_.array))[2].AsFloat(), (*(data_.array))[3].AsFloat(),
            (*(data_.array))[4].AsFloat(), (*(data_.array))[5].AsFloat(), (*(data_.array))[6].AsFloat(), (*(data_.array))[7].AsFloat(),
            (*(data_.array))[8].AsFloat(), (*(data_.array))[9].AsFloat(), (*(data_.array))[10].AsFloat(), (*(data_.array))[11].AsFloat(),
            (*(data_.array))[12].AsFloat(), (*(data_.array))[13].AsFloat(), (*(data_.array))[14].AsFloat(), (*(data_.array))[15].AsFloat());
    }
    return Matrix4x4();
}

std::string Json::JsonValue::AsString() const
{
    switch (type_) {
    case Json::JSON_NULL:
        return std::string("null");
        break;
    case Json::JSON_BOOLEAN:
        return std::to_string(data_.boolean);
        break;
    case Json::JSON_NUMBER:
        return std::to_string(data_.number);
        break;
    case Json::JSON_STRING:
        return std::string(data_.str);
        break;
    case Json::JSON_OBJECT:
        return std::string("JSON_OBJECT");
        break;
    case Json::JSON_ARRAY:
    default:
        return std::string("JSON_ARRAY SIZE = " + std::to_string(type_));
    }
    return std::string();
}

const Json::JsonValue* Json::JsonValue::AsJsonArray() const
{
    if (type_ >= JSON_ARRAY) {
        return (*(data_.array)).data();
    }
    return &JsonValue::NULL_OBJECT;
}

const Json::JsonObject* Json::JsonValue::AsJsonObject() const
{
    if (type_ == JSON_OBJECT) {
        return data_.object;
    }
    return nullptr;
}

Json::JsonObject::~JsonObject()
{
    for (auto&& keyValue : valueTable_) {
        keyValue.second.Destroy();
    }
}

const Json::JsonValue& Json::JsonObject::GetValue(const std::string& key) const
{
    auto valueIter = valueTable_.find(key);
    if (valueIter != valueTable_.end()) {
        return valueIter->second;
    }
    return JsonValue::NULL_OBJECT;
}

Json::Parser::Parser(JsonObject* jsonObject, const char* json, const size_t length)
    : json_(json), length_(length), jsonObject_(jsonObject)
{
    Parse();
}

Json::Parser::Parser(JsonObject* jsonObject, const std::string& json)
    : json_(json.c_str()), length_(json.length()), jsonObject_(jsonObject)
{
    Parse();
}

void Json::Parser::Parse()
{
    ch_ = json_[0];
    SkipSpacesAndComments();
    if (ch_ != '{') {
        SubmitError();
    } else {
        ParseObject(jsonObject_);
    }
}

void Json::Parser::SubmitError()
{
    if (!hasError_) {
        hasError_ = true;
        errorMessage_ += "Error Around Row: " + std::to_string(row_) + ", column: " + std::to_string(column_);
    }
}
bool Json::Parser::HasError()
{
    return hasError_;
}

std::string Json::Parser::GetErrorMessage() const
{
    return errorMessage_;
}

bool Json::Parser::Forward()
{
    if (finish_ || hasError_) {
        return false;
    }
    if (ch_ == '\n') {
        column_ = 1;
        row_++;
    } else {
        column_++;
    }
    offset_++;
    if (offset_ >= length_) {
        ch_ = '\0';
        return false;
    }
    ch_ = json_[offset_];
    if (ch_ == '\0') {
        SubmitError();
        return false;
    }
    return true;
}

void Json::Parser::Forward(size_t count)
{
    for (size_t i = 0; i < count; i++) {
        Forward();
    }
}

bool Json::Parser::SkipSpaces()
{
    bool found = false;
    if (std::isspace(ch_)) {
        found = true;
        while (Forward() && std::isspace(ch_)) {}
    }
    return found;
}

bool Json::Parser::SkipComments()
{
    bool found = false;
    if (ch_ == '/') {
        if (Forward() && ch_ == '/') {
            found = true;
            int nextRow = row_ + 1;
            while (Forward() && row_ < nextRow) {}
        }
    }
    return found;
}

void Json::Parser::SkipSpacesAndComments()
{
    while (SkipSpaces() || SkipComments()) {
        // break until there are no spaces and comments
    }
}

void Json::Parser::ParseObject(JsonObject* object)
{
    if (ch_ != '{') {
        SubmitError();
        return;
    }
    while (Forward() && ch_ != '}') {
        SkipSpacesAndComments();
        if (ch_ == '"') {
            auto key = ParseString();
            if (hasError_) {
                break;
            }
            if (object->valueTable_.find(key) != object->valueTable_.end()) {
                SubmitError();
                break;
            }
            SkipSpacesAndComments();
            if (ch_ == ':') {
                if (Forward()) {
                    SkipSpacesAndComments();
                    JsonValue value;
                    ParseValue(value);
                    if (hasError_) {
                        break;
                    }
                    object->valueTable_.emplace(std::make_pair(key, value));
                    SkipSpacesAndComments();
                    if (ch_ == ',') {
                        // parse next value then
                    } else if (ch_ == '}') {
                        Forward();
                        break;
                    } else {
                        SubmitError();
                        break;
                    }
                } else {
                    SubmitError();
                    break;
                }
            } else {
                SubmitError();
                break;
            }
        } else {
            SubmitError();
            break;
        }
    }
}

void Json::Parser::ParseBoolean(JsonValue& value)
{
    if (ch_ == 't') {
        if (Forward() && ch_ == 'r') {
            if (Forward() && ch_ == 'u') {
                if (Forward() && ch_ == 'e') {
                    value.type_ = JSON_BOOLEAN;
                    value.data_.boolean = true;
                    Forward();
                    return;
                }
            }
        }
    } else if (ch_ == 'f') {
        if (Forward() && ch_ == 'a') {
            if (Forward() && ch_ == 'l') {
                if (Forward() && ch_ == 's') {
                    if (Forward() && ch_ == 'e') {
                        value.type_ = JSON_BOOLEAN;
                        value.data_.boolean = false;
                        Forward();
                        return;
                    }
                }
            }
        }
    }
    SubmitError();
}

void Json::Parser::ParseValue(JsonValue& value)
{
    if (ch_ == '"') {
        auto str = ParseString();
        if (hasError_) {
            return;
        } else {
            std::cerr << "Alloc" << std::endl;
            char* data = new char[str.length() + 1];
            str.copy(data, str.length());
            data[str.length()] = '\0';
            value.data_.str = data;
            value.type_ = JSON_STRING;
        }
    } else if (ch_ == '-' || ch_ == '+' || std::isdigit(ch_)) {
        ParseNumber(value);
    } else if (ch_ == 't' || ch_ == 'f') {
        ParseBoolean(value);
    } else if (ch_ == '[') {
        ParseArray(value);
    } else if (ch_ == '{') {
        JsonObject* object = new JsonObject;
        std::cerr << "Alloc" << std::endl;
        ParseObject(object);
        value.type_ = JSON_OBJECT;
        value.data_.object = object;
    }
}

void Json::Parser::ParseNumber(JsonValue& value)
{
    size_t begin = offset_;
    if (ch_ == '-' || ch_ == '+') {
        Forward();
    }
    if (ch_ > '0' && ch_ <= '9') {
        while (Forward() && std::isdigit(ch_)) {}
    } else {
        Forward();
    }
    if (ch_ == '.') {
        while (Forward() && std::isdigit(ch_)) {}
    }
    if (ch_ == 'e' || ch_ == 'E') {
        if (Forward()) {
            if (ch_ != '+' && ch_ != '-' && !std::isdigit(ch_)) {
                SubmitError();
                return;
            }
            while (Forward() && std::isdigit(ch_)) {}
        }
    }
    value.data_.number = std::stof(std::string(json_ + begin, json_ + offset_));
    value.type_ = JSON_NUMBER;
}

void Json::Parser::ParseArray(JsonValue& value)
{
    if (ch_ != '[') {
        SubmitError();
        return;
    }
    Forward();
    SkipSpacesAndComments();
    std::cerr << "Alloc" << std::endl;
    std::vector<JsonValue>* array = new std::vector<JsonValue>();
    while (ch_ != ']') {
        array->emplace_back();
        ParseValue(array->back());
        if (hasError_) {
            break;
        }
        SkipSpacesAndComments();
        if (ch_ == ',') {
            if (Forward()) {
                SkipSpacesAndComments();
            } else {
                SubmitError();
                break;
            }
        } else if (ch_ != ']') {
            SubmitError();
            break;
        }
    }
    if (hasError_) {
        for (auto&& value : *array) {
            value.Destroy();
        }
        delete array;
        std::cerr << "De-Alloc" << std::endl;
    } else {
        if (array->empty()) {
            delete array;
            std::cerr << "De-Alloc" << std::endl;
            value.type_ = JSON_NULL;
            value.data_.array = nullptr;
        } else {
            Forward();
            value.type_ = static_cast<ValueType>(JSON_ARRAY + array->size());
            value.data_.array = array;
        }
    }
}

std::string Json::Parser::ParseString()
{
    if (ch_ != '"') {
        SubmitError();
        return "";
    }
    auto begin = offset_;
    auto end = begin + 1;
    while (end < length_ && json_[end] != '"') {
        end++;
    }
    if (end < length_ && json_[end] == '"') {
        end++;
        Forward(end - begin);
        if (begin + 2 == end) {
            return "";
        }
        return std::string(json_ + begin + 1, json_ + end - 1);
    }
    SubmitError();
    return "";
}
}
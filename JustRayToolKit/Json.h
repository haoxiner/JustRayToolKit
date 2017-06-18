#pragma once
#include "MathUtil.h"
#include <string>
#include <vector>
#include <map>
#include <utility>
namespace JustRay
{
namespace Json
{
enum ValueType
{
    JSON_NULL = -5,
    JSON_BOOLEAN = -4,
    JSON_NUMBER = -3,
    JSON_STRING = -2,
    JSON_OBJECT = -1,
    JSON_ARRAY = 0
};
class JsonObject;
class JsonValue
{
    friend class Parser;
public:
    JsonValue();
    JsonValue(float number);
    JsonValue(bool boolean);
    JsonValue(char* str);
    JsonValue(JsonObject* object);
    void Destroy();
    int AsInt() const;
    bool AsBool() const;
    float AsFloat() const;
    Float2 AsFloat2() const;
    Float3 AsFloat3() const;
    Float4 AsFloat4() const;
    Matrix4x4 AsMatrix4x4() const;
    std::string AsString() const;
    const JsonValue* AsJsonArray() const;
    const JsonObject* AsJsonObject() const;
    static const JsonValue NULL_OBJECT;
private:
    union Value
    {
        Value(bool value) : boolean(value) {}
        Value(float value) : number(value) {}
        Value(char* value) : str(value) {}
        Value(JsonObject* value) : object(value) {}
        bool boolean;
        float number;
        char* str;
        JsonObject* object;
        std::vector<JsonValue>* array;
    };
    Value data_ = { false };
    ValueType type_ = JSON_NULL;
};
class Parser
{
public:
    Parser(JsonObject* jsonObject, const char* json, const size_t length);
    Parser(JsonObject* jsonObject, const std::string& json);
    bool HasError();
    std::string GetErrorMessage() const;
private:
    void Parse();
    void SubmitError();
    bool Forward();
    void Forward(size_t count);
    bool SkipSpaces();
    bool SkipComments();
    void SkipSpacesAndComments();

    void ParseObject(JsonObject* object);
    void ParseBoolean(JsonValue& value);
    void ParseValue(JsonValue& value);
    void ParseNumber(JsonValue& value);
    void ParseArray(JsonValue& value);
    std::string ParseString();

    const char* json_ = nullptr;
    const size_t length_ = 0;
    size_t offset_ = 0;
    JsonObject* jsonObject_;
    char ch_ = 0;
    bool hasError_ = false;
    std::string errorMessage_;
    bool finish_ = false;
    size_t row_ = 1;
    size_t column_ = 1;
};
class JsonObject
{
    friend class Parser;
public:
    ~JsonObject();
    const JsonValue& GetValue(const std::string& key) const;
private:
    std::map<std::string, JsonValue> valueTable_;
};
}
}
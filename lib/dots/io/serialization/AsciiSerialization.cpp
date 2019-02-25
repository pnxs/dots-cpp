#include "AsciiSerialization.h"
#include "dots/type/Registry.h"

#include <stack>
#include <iostream>

namespace dots {

struct Printer {
    virtual ~Printer() = default;
    virtual void StartObject() = 0;
    virtual void EndObject() = 0;
    virtual void StartArray() = 0;
    virtual void EndArray() = 0;
    virtual void String(const string& s) = 0;
    virtual void Bool(bool b) = 0;
    virtual void Int(int i) = 0;
    virtual void Uint(unsigned int i) = 0;
    virtual void Int64(int64_t i) = 0;
    virtual void Uint64(uint64_t i) = 0;
    virtual void Double(double d) = 0;
    virtual void Null() = 0;
    virtual void Enum(const void* e, const type::EnumDescriptor* ed) = 0;
    virtual std::string GetString() const = 0;
    virtual void BeginHighlight() = 0;
    virtual void EndHightlight() = 0;
};

struct PrettyPrinter: public Printer
{
    enum class write_mode { plain, object_key, object_value, array };

    void StartObject() override {
        if (m_writeMode.empty()) {
            m_writeMode.push(write_mode::plain);
        }
        writeValue("{");
        m_writeMode.push(write_mode::object_key);
        m_indentLevel++;
    }

    void EndObject() override {
        m_indentLevel--;
        m_writeMode.pop();
        m_line = "}";
        if (m_writeMode.top() == write_mode::array)
            m_line += ",";
        addLine(m_line);
        m_line.clear();
    }

    void StartArray() override {
        writeValue("[");
        m_writeMode.push(write_mode::array);
        m_indentLevel++;
    }

    void EndArray() override {
        m_indentLevel--;
        m_writeMode.pop();
        addLine("]");
    }

    void String(const string& s) override {
        writeValue(s);
    }

    void Bool(bool b) override {
        writeValue(std::to_string(b));
    }

    void Int(int i) override {
        writeValue(std::to_string(i));
    }

    void Uint(unsigned int i) override {
        writeValue(std::to_string(i));
    }

    void Int64(int64_t i) override {
        writeValue(std::to_string(i));
    }

    void Uint64(uint64_t i) override {
        writeValue(std::to_string(i));
    }

    void Double(double d) override {
        writeValue(std::to_string(d));
    }

    void Null() override {
        writeValue("'null'");
    }

    void Enum(const void* e, const type::EnumDescriptor* ed) override {
        //auto enumValue = enumDescriptor->to_int(data);
        //writer.Int(enumDescriptor->value2key(enumValue));
        writeValue(ed->to_string(e));
    }

    void BeginHighlight() override {}
    void EndHightlight() override {}

    std::string GetString() const override { return m_buffer; }

private:
    void writeValue(const std::string& s)
    {
        switch(m_writeMode.top())
        {
            case write_mode::plain:
                m_line += s;
                addLine(m_line);
                m_line.clear();
                break;
            case write_mode::object_key:
                m_line += s + "=";
                m_writeMode.pop();
                m_writeMode.push(write_mode::object_value);
                break;
            case write_mode::object_value:
                m_line += s;
                addLine(m_line);
                m_line.clear();
                m_writeMode.pop();
                m_writeMode.push(write_mode::object_key);
                break;
            case write_mode::array:
                addLine(s);
                break;
        }
    }

    void addLine(const std::string& line)
    {
        std::string::size_type charCount = m_indentLevel * m_indentation;
        std::string spaces(charCount, ' ');

        m_buffer += spaces + line + "\n";
    }

    std::string m_line;
    bool m_objectValue = false;
    std::stack<write_mode> m_writeMode;
    uint16_t m_indentLevel = 0;
    uint16_t m_indentation = 4;
    std::string m_buffer;
};

struct SingleLinePrinter: public Printer
{
    enum class write_mode { plain, object_key, object_value, array };

    SingleLinePrinter(const ToAsciiOptions& options)
            :m_cs(options.cs), enumAsTag(options.enumAsTag)
    {
    }

    void StartObject() override {
        if (m_writeMode.empty()) {
            m_writeMode.push(write_mode::plain);
        }
        m_result += "<";
        if (m_writeMode.top() == write_mode::object_value)
            m_writeMode.top() = write_mode::object_key;
        m_writeMode.push(write_mode::object_key);
    }

    void EndObject() override {
        m_result += ">";
        m_writeMode.pop();
        if (m_writeMode.top() == write_mode::array)
            m_result += ", ";
    }

    void StartArray() override {
        m_result += "[";
        if (m_writeMode.top() == write_mode::object_value)
            m_writeMode.top() = write_mode::object_key;
        m_writeMode.push(write_mode::array);
    }

    void EndArray() override {
        m_result.erase(m_result.length()-2); // strip ", "
        m_result += "] ";
        m_writeMode.pop();
        if (m_writeMode.top() == write_mode::array)
            m_result += ", ";
    }

    void String(const string& s) override {
        if (m_cs && (m_writeMode.top() != write_mode::object_key)) m_result += m_cs->string();
        writeValue(s);
    }

    void Bool(bool b) override {
        if (m_cs) m_result += m_cs->integer();
        writeValue(std::to_string(b));
    }

    void Int(int i) override {
        if (m_cs) m_result += m_cs->integer();
        writeValue(std::to_string(i));
    }

    void Uint(unsigned int i) override {
        if (m_cs) m_result += m_cs->integer();
        writeValue(std::to_string(i));
    }

    void Int64(int64_t i) override {
        if (m_cs) m_result += m_cs->integer();
        writeValue(std::to_string(i));
    }

    void Uint64(uint64_t i) override {
        if (m_cs) m_result += m_cs->integer();
        writeValue(std::to_string(i));
    }

    void Double(double d) override {
        if (m_cs) m_result += m_cs->floatingPoint();
        writeValue(std::to_string(d));
    }

    void Null() override {
        writeValue("'null'");
    }

    void Enum(const void* e, const type::EnumDescriptor* ed) override {
        if (enumAsTag) {
            auto enumValue = ed->to_int(e);
            Int(ed->value2key(enumValue));
        }
        else {
            if (m_cs) m_result += m_cs->enumValue();
            writeValue(ed->to_string(e));
        }
    }

    void BeginHighlight() override {
        if (m_cs) {
            m_result += m_cs->highlight();
        }
    }

    void EndHightlight() override {

    }

    std::string GetString() const override { return m_result; }

private:
    void writeValue(const std::string& s)
    {
        switch(m_writeMode.top())
        {
            case write_mode::plain:
                m_result += s;
                if (m_cs) m_result += m_cs->allOff();
                break;
            case write_mode::object_key:
                if (m_cs) {
                    m_cs->attribute();
                }
                m_result += s;
                if (m_cs) m_result += m_cs->allOff();
                m_result += ":";
                m_writeMode.top() = write_mode::object_value;
                break;
            case write_mode::object_value:
                m_result += s + " ";
                if (m_cs) m_result += m_cs->allOff();
                m_writeMode.top() = write_mode::object_key;
                break;
            case write_mode::array:
                m_result += s;
                if (m_cs) m_result += m_cs->allOff();
                m_result += ", ";
                break;
        }
    }

    std::string m_result;
    std::stack<write_mode> m_writeMode;
    std::string m_buffer;
    const ToAsciiColorSchema* m_cs = nullptr;
    bool enumAsTag = false;
};

static void to_ascii_recursive(const dots::type::StructDescriptor *td, const void *data, property_set what, Printer& writer, property_set highlight);
static void write_array_to_ascii(const type::VectorDescriptor* vd, const void* data, Printer& writer);

static void write_atomic_types_to_ascii(const type::Descriptor* td, const void* data, Printer& writer)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    switch (td->dotsType()) {
        case type::DotsType::int8:            writer.Int(*(const int8_t *) data); break;
        case type::DotsType::int16:           writer.Int(*(const int16_t *) data); break;
        case type::DotsType::int32:           writer.Int(*(const int32_t *) data); break;
        case type::DotsType::int64:           writer.Int64(*(const long long *) data); break;
        case type::DotsType::uint8:           writer.Uint(*(const uint8_t *) data); break;
        case type::DotsType::uint16:          writer.Uint(*(const uint16_t *) data); break;
        case type::DotsType::uint32:          writer.Uint(*(const uint32_t *) data); break;
        case type::DotsType::uint64:          writer.Uint64(*(const unsigned long long *) data); break;
        case type::DotsType::boolean:         writer.Bool(*(const bool *) data); break;
        case type::DotsType::float16:         writer.Double(*(const float *) data); break;
        case type::DotsType::float32:         writer.Double(*(const float *) data);break;
        case type::DotsType::float64:         writer.Double(*(const double *) data);break;
        case type::DotsType::string:          writer.String(*(const std::string*) data);break;
        case type::DotsType::property_set:    writer.Int(((const dots::property_set*)data)->value()); break;
        case type::DotsType::timepoint:       writer.Double(((const pnxs::TimePoint*)data)->value()); break;
        case type::DotsType::steady_timepoint:writer.Double(((const pnxs::SteadyTimePoint*)data)->value()); break;
        case type::DotsType::duration:        writer.Double(*(const pnxs::Duration*)data); break;
        case type::DotsType::uuid:            writer.String(((const dots::uuid*)data)->toString()); break;
        case type::DotsType::Enum:
        {
            auto enumDescriptor = dynamic_cast<const type::EnumDescriptor*>(td);
            writer.Enum(data, enumDescriptor);
        }
            break;
        case type::DotsType::Vector:
        case type::DotsType::Struct:

            throw std::runtime_error("unknown type: " + td->name());
        case type::DotsType::pointer:
            throw std::runtime_error("unable to serialize pointer-type");
    }
}


static inline void write_ascii(const type::Descriptor* td, const void* data, Printer& writer)
{
    if (td->dotsType() == type::DotsType::Vector)
    {
        auto vectorDescriptor = type::toVectorDescriptor(td);
        write_array_to_ascii(vectorDescriptor, data, writer);
    }
    else if (isDotsBaseType(td->dotsType()))
    {
        write_atomic_types_to_ascii(td, data, writer);
    }
    else if (td->dotsType() == type::DotsType::Struct) // object
    {
        auto structDescriptor = type::toStructDescriptor(td);
        to_ascii_recursive(structDescriptor, data, PROPERTY_SET_ALL, writer, PROPERTY_SET_NONE);
    }
    else
    {
        throw std::runtime_error("unable to decode array");
    }
}

static void write_array_to_ascii(const type::VectorDescriptor* vd, const void* data, Printer& writer)
{
    auto vectorSize = vd->get_size(data);
    writer.StartArray();

    auto itemDescriptor = vd->vtd();

    for (unsigned int i = 0; i < vectorSize; ++i)
    {
        write_ascii(itemDescriptor, vd->get_data(data, i), writer);
    }

    writer.EndArray();
}

static void to_ascii_recursive(const dots::type::StructDescriptor *td, const void *data, property_set what, Printer& writer, property_set highlight)
{
    property_set validProperties = td->validProperties(data);

    /// attributes which should be serialized 'and' are valid
    const property_set serializePropertySet = (what & validProperties);

    //size_t nrElements = serializePropertySet.count();

    writer.StartObject();

    auto prop_list = td->properties();
    for (auto prop : prop_list)
    {
        auto tag = prop.tag();

        if (not serializePropertySet.test(tag))
            continue;

        auto propertyValue = prop.address(data);

        //std::cout << "cbor write property '" << prop.name() << "' tag: " << tag << ":\n";

        const std::string name = prop.name().data();

        if (highlight.test(tag)) {
            writer.BeginHighlight();
        }

        writer.String(name);

        if (highlight.test(tag)) {
            writer.EndHightlight();
        }

        write_ascii(prop.td(), propertyValue, writer);
    }

    writer.EndObject();
}


std::string to_ascii(const dots::type::StructDescriptor *td, const void *data, property_set properties, const ToAsciiOptions& cs)
{
    std::unique_ptr<Printer> printer;

    if (cs.singleLine) {
        printer = std::make_unique<SingleLinePrinter>(cs);
    }
    else {
        printer = std::make_unique<PrettyPrinter>();
    }

    to_ascii_recursive(td, data, properties, *printer.get(), cs.highlightAttributes);

    return printer->GetString();
}

}
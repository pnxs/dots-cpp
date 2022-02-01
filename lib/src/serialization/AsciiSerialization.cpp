#include <dots/serialization/AsciiSerialization.h>
#include <dots/type/Struct.h>
#include <dots/type/EnumDescriptor.h>
#include <stack>

namespace dots {

struct Printer {
    virtual ~Printer() = default;
    virtual void StartObject() = 0;
    virtual void EndObject() = 0;
    virtual void StartArray() = 0;
    virtual void EndArray() = 0;
    virtual void String(const std::string& s) = 0;
    virtual void Bool(bool b) = 0;
    virtual void Int(int i) = 0;
    virtual void Uint(unsigned int i) = 0;
    virtual void Int64(int64_t i) = 0;
    virtual void Uint64(uint64_t i) = 0;
    virtual void Double(double d) = 0;
    virtual void Null() = 0;
    virtual void Enum(const void* e, const type::EnumDescriptor<>* ed) = 0;
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

    void String(const std::string& s) override {
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

    void Enum(const void* e, const type::EnumDescriptor<>* ed) override {
        //auto enumValue = enumDescriptor->to_int(data);
        //writer.Int(enumDescriptor->value2key(enumValue));
        writeValue(ed->enumeratorFromValue(*type::Typeless::From(e)).name());
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

    void String(const std::string& s) override {
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

    void Enum(const void* e, const type::EnumDescriptor<>* ed) override {
        if (enumAsTag) {
            Int(ed->enumeratorFromValue(*type::Typeless::From(e)).tag());
        }
        else {
            if (m_cs) m_result += m_cs->enumValue();
            writeValue(ed->enumeratorFromValue(*type::Typeless::From(e)).name());
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

static void to_ascii_recursive(const dots::type::StructDescriptor<>& td, const void *data, types::property_set_t what, Printer& writer, types::property_set_t highlight);
static void write_array_to_ascii(const type::VectorDescriptor& vd, const type::Vector<>& data, Printer& writer);

static void write_atomic_types_to_ascii(const type::Descriptor<>& td, const void* data, Printer& writer)
{
    //std::cout << "write atomic is_arithmetic:" << t.is_arithmetic() << " is_enum:" << t.is_enumeration() << " t:" << t.get_name() << "\n";
    //std::cout << "var ptr: " << var.get_ptr() << " type=" << var.get_type().get_name() << "\n";
    switch (td.type()) {
        case type::Type::int8:            writer.Int(*static_cast<const int8_t*>(data)); break;
        case type::Type::int16:           writer.Int(*static_cast<const int16_t*>(data)); break;
        case type::Type::int32:           writer.Int(*static_cast<const int32_t*>(data)); break;
        case type::Type::int64:           writer.Int64(*static_cast<const long long*>(data)); break;
        case type::Type::uint8:           writer.Uint(*static_cast<const uint8_t*>(data)); break;
        case type::Type::uint16:          writer.Uint(*static_cast<const uint16_t*>(data)); break;
        case type::Type::uint32:          writer.Uint(*static_cast<const uint32_t*>(data)); break;
        case type::Type::uint64:          writer.Uint64(*static_cast<const unsigned long long*>(data)); break;
        case type::Type::boolean:         writer.Bool(*static_cast<const bool*>(data)); break;
        case type::Type::float32:         writer.Double(*static_cast<const float*>(data));break;
        case type::Type::float64:         writer.Double(*static_cast<const double*>(data));break;
        case type::Type::string:          writer.String(*static_cast<const std::string*>(data));break;
        case type::Type::property_set:    writer.Int(static_cast<const dots::types::property_set_t*>(data)->toValue()); break;
        case type::Type::timepoint:       writer.Double(static_cast<const type::TimePoint*>(data)->duration().toFractionalSeconds()); break;
        case type::Type::steady_timepoint:writer.Double(static_cast<const type::SteadyTimePoint*>(data)->duration().toFractionalSeconds()); break;
        case type::Type::duration:        writer.Double(static_cast<const type::Duration*>(data)->toFractionalSeconds()); break;
        case type::Type::uuid:            writer.String(static_cast<const dots::types::uuid_t*>(data)->toString()); break;
        case type::Type::Enum:
        {
            writer.Enum(data, static_cast<const type::EnumDescriptor<>*>(&td));
        }
            break;
        case type::Type::Vector:
        case type::Type::Struct:

            throw std::runtime_error("unknown type: " + td.name());
    }
}


static inline void write_ascii(const type::Descriptor<>& td, const void* data, Printer& writer)
{
    if (td.type() == type::Type::Vector)
    {
        write_array_to_ascii(static_cast<const type::VectorDescriptor&>(td), *static_cast<const type::Vector<>*>(data), writer);
    }
    else if (td.isFundamentalType() || td.type() == type::Type::Enum)
    {
        write_atomic_types_to_ascii(td, data, writer);
    }
    else if (td.type() == type::Type::Struct) // object
    {
        to_ascii_recursive(static_cast<const type::StructDescriptor<>&>(td), data, types::property_set_t::All, writer, types::property_set_t::None);
    }
    else
    {
        throw std::runtime_error("unable to decode array");
    }
}

static void write_array_to_ascii(const type::VectorDescriptor& vd, const type::Vector<>& data, Printer& writer)
{
    writer.StartArray();

    for (unsigned int i = 0; i < data.typelessSize(); ++i)
    {
        write_ascii(vd.valueDescriptor(), reinterpret_cast<const void*>(&data.typelessAt(i)), writer);
    }

    writer.EndArray();
}

static void to_ascii_recursive(const type::StructDescriptor<>& td, const void *data, types::property_set_t what, Printer& writer, types::property_set_t highlight)
{
    types::property_set_t validProperties = td.propertyArea(*static_cast<const type::Struct*>(data)).validProperties();

    /// attributes which should be serialized 'and' are valid
    const types::property_set_t serializePropertySet = (what ^ validProperties);

    //size_t nrElements = serializePropertySet.count();

    writer.StartObject();

    const auto& instance = type::Typeless::From(data)->to<type::Struct>();

    for (const auto& property : instance._propertyRange(serializePropertySet))
    {
        if (property.descriptor().set() <= highlight)
        {
            writer.BeginHighlight();
            writer.String(property.descriptor().name());
            writer.EndHightlight();
        }
        else
        {
            writer.String(property.descriptor().name());
        }

        write_ascii(property.descriptor().valueDescriptor(), &property.value(), writer);
    }

    writer.EndObject();
}


std::string to_ascii(const type::StructDescriptor<>* td, const void* data, types::property_set_t properties/* = types::property_set_t::All*/, const ToAsciiOptions& cs/* = {}*/)
{
    std::unique_ptr<Printer> printer;

    if (cs.singleLine) {
        printer = std::make_unique<SingleLinePrinter>(cs);
    }
    else {
        printer = std::make_unique<PrettyPrinter>();
    }

    to_ascii_recursive(*td, data, properties, *printer.get(), cs.highlightAttributes);

    return printer->GetString();
}

}

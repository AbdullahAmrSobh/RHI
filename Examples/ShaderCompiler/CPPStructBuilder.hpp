#pragma once

#include <TL/String.hpp>
#include <TL/Context.hpp>
#include <TL/Containers.hpp>
#include <TL/Block.hpp>
#include <TL/UniquePtr.hpp>
#include <TL/Span.hpp>

#include <cstdint>
#include <cstddef>

namespace CPPStructBuilder
{
    // Helper struct for struct fields: type and name
    struct StructField
    {
        TL::Ref<struct Type> type;
        TL::String           name;
    };

    // Helper struct for enum fields: name and value
    struct EnumField
    {
        TL::String name;
        uint64_t   value;
    };

    struct Type
    {
        enum class Kind
        {
            None,
            Int,
            Uint,
            Float,
            Boolean,
            Enum,
            Struct,
            Vector,
            Matrix,
            Array,
        };

    private:
        TL::String name;
        Kind       kind      = Kind::None;
        uint32_t   size      = 0;
        uint32_t   alignment = 0;

        // For composite types
        TL::Map<TL::Ref<Type>, TL::String> structFields;
        TL::Map<TL::String, uint64_t>      enumFields;

        // For vector/matrix/array types
        TL::Ref<Type> elementType;
        uint32_t      elementCount = 0; // For vector/array: number of elements; for matrix: columns

        uint32_t rowCount = 0; // For matrix: number of rows

    public:
        // Default constructor
        Type() = default;

        // Constructor for primitive types
        Type(const TL::String& n, Kind k, uint32_t s, uint32_t a)
            : name(n)
            , kind(k)
            , size(s)
            , alignment(a)
        {
        }

        // Constructor for struct types from StructField span
        Type(const TL::String& name, TL::Span<const StructField> fields)
        {
            this->name = name;
            this->kind = Kind::Struct;
            for (const auto& field : fields)
            {
                structFields[field.type] = field.name;
            }
            // Size/alignment calculation can be added here
        }

        // Constructor for enum types from EnumField span
        Type(const TL::String& name, TL::Span<const EnumField> fields)
        {
            this->name = name;
            this->kind = Kind::Enum;
            for (const auto& field : fields)
            {
                enumFields[field.name] = field.value;
            }
            // Size/alignment calculation can be added here
        }

        // Helper for integer to string conversion (since TL::ToString is missing)
        static std::string IntToString(uint32_t value)
        {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%u", value);
            return std::string(buffer);
        }

        // Constructor for vector types as struct with x/y/z/w...
        Type(const TL::String& name, TL::Ref<Type> elemType, uint32_t count)
        {
            this->name = name;
            this->kind = Kind::Struct;
            static const char* swizzle[] = {"x", "y", "z", "w", "v", "u", "s", "t"};
            std::vector<StructField> fields;
            for (uint32_t i = 0; i < count; ++i)
            {
                TL::String fieldName = (i < 8) ? swizzle[i] : ("e" + TL::String(IntToString(i).c_str()));
                fields.push_back({elemType, fieldName});
            }
            for (const auto& field : fields)
            {
                structFields[field.type] = field.name;
            }
            this->size         = elemType->GetSize() * count;
            this->alignment    = elemType->GetAlignment();
            this->elementType  = elemType;
            this->elementCount = count;
        }

        // Constructor for matrix types as struct of vectors
        Type(const TL::String& name, TL::Ref<Type> elemType, uint32_t columns, uint32_t rows)
        {
            this->name = name;
            this->kind = Kind::Struct;
            std::vector<StructField> fields;
            for (uint32_t i = 0; i < columns; ++i)
            {
                TL::String fieldName = "col" + TL::String(IntToString(i).c_str());
                TL::Ref<Type> vecType = TL::CreateRef<Type>("vec" + TL::String(IntToString(rows).c_str()), elemType, rows);
                fields.push_back({vecType, fieldName});
            }
            for (const auto& field : fields)
            {
                structFields[field.type] = field.name;
            }
            this->size         = elemType->GetSize() * columns * rows;
            this->alignment    = elemType->GetAlignment();
            this->elementType  = elemType;
            this->elementCount = columns;
            this->rowCount     = rows;
        }

        // Constructor for fixed-size array types
        Type(const TL::String& name, TL::Ref<Type> elemType, uint32_t count, Kind arrayKind)
        {
            this->name         = name;
            this->kind         = arrayKind;
            this->elementType  = elemType;
            this->elementCount = count;
            this->size         = elemType->GetSize() * count;
            this->alignment    = elemType->GetAlignment();
        }

        // Getters
        const TL::String& GetName() const { return name; }

        Kind GetKind() const { return kind; }

        uint32_t GetSize() const { return size; }

        uint32_t GetAlignment() const { return alignment; }

        const TL::Map<TL::Ref<Type>, TL::String>& GetStructFields() const
        {
            TL_ASSERT(kind == Kind::Struct, "GetStructFields called on non-struct type");
            return structFields;
        }

        const TL::Map<TL::String, uint64_t>& GetEnumFields() const
        {
            TL_ASSERT(kind == Kind::Enum, "GetEnumFields called on non-enum type");
            return enumFields;
        }

        TL::Ref<Type> GetElementType() const
        {
            TL_ASSERT(kind == Kind::Vector || kind == Kind::Matrix || kind == Kind::Array, "GetElementType called on non-composite type");
            return elementType;
        }

        uint32_t GetElementCount() const
        {
            TL_ASSERT(kind == Kind::Vector || kind == Kind::Array || kind == Kind::Matrix, "GetElementCount called on non-composite type");
            return elementCount;
        }

        uint32_t GetRowCount() const
        {
            TL_ASSERT(kind == Kind::Matrix, "GetRowCount called on non-matrix type");
            return rowCount;
        }

        // Setters
        void SetName(const TL::String& n) { name = n; }

        void SetKind(Kind k) { kind = k; }

        void SetSize(uint32_t s) { size = s; }

        void SetAlignment(uint32_t a) { alignment = a; }

        void SetStructFields(const TL::Map<TL::Ref<Type>, TL::String>& fields)
        {
            TL_ASSERT(kind == Kind::Struct, "SetStructFields called on non-struct type");
            structFields = fields;
        }

        void SetEnumFields(const TL::Map<TL::String, uint64_t>& fields)
        {
            TL_ASSERT(kind == Kind::Enum, "SetEnumFields called on non-enum type");
            enumFields = fields;
        }

        void SetElementType(TL::Ref<Type> elemType)
        {
            TL_ASSERT(kind == Kind::Vector || kind == Kind::Matrix || kind == Kind::Array, "SetElementType called on non-composite type");
            elementType = elemType;
        }

        void SetElementCount(uint32_t count)
        {
            TL_ASSERT(kind == Kind::Vector || kind == Kind::Array || kind == Kind::Matrix, "SetElementCount called on non-composite type");
            elementCount = count;
        }

        void SetRowCount(uint32_t rows)
        {
            TL_ASSERT(kind == Kind::Matrix, "SetRowCount called on non-matrix type");
            rowCount = rows;
        }

        // Static Create methods
        static TL::Ref<Type> Create(const TL::String& n, Kind k, uint32_t s, uint32_t a)
        {
            return TL::CreateRef<Type>(n, k, s, a);
        }

        static TL::Ref<Type> Create(const TL::String& name, TL::Span<const StructField> fields)
        {
            return TL::CreateRef<Type>(name, fields);
        }

        static TL::Ref<Type> Create(const TL::String& name, TL::Span<const EnumField> fields)
        {
            return TL::CreateRef<Type>(name, fields);
        }

        static TL::Ref<Type> CreateVector(const TL::String& name, TL::Ref<Type> elemType, uint32_t count)
        {
            return TL::CreateRef<Type>(name, elemType, count);
        }

        static TL::Ref<Type> CreateMatrix(const TL::String& name, TL::Ref<Type> elemType, uint32_t columns, uint32_t rows)
        {
            return TL::CreateRef<Type>(name, elemType, columns, rows);
        }

        static TL::Ref<Type> CreateArray(const TL::String& name, TL::Ref<Type> elemType, uint32_t count)
        {
            return TL::CreateRef<Type>(name, elemType, count, Kind::Array);
        }

        // Returns a string containing the C++ definition of the type
        TL::String ToCppDefinition() const
        {
            switch (kind)
            {
            case Kind::Struct:
                {
                    TL::String result = "struct " + name + "\n{\n";
                    for (const auto& [fieldType, fieldName] : structFields)
                    {
                        result += "    " + fieldType->GetName() + " " + fieldName + ";\n";
                    }
                    result += "};\n";
                    return result;
                }
            case Kind::Enum:
                {
                    TL::String result = "enum class " + name + "\n{\n";
                    for (const auto& [enumName, enumValue] : enumFields)
                    {
                        // result += "    " + enumName + " = " + TL::ToString(enumValue) + ",\n";
                    }
                    result += "};\n";
                    return result;
                }
            case Kind::Vector:
                // Not used, vectors are now structs
                return "";
            case Kind::Matrix:
                // Not used, matrices are now structs
                return "";
            case Kind::Array:
                return elementType->GetName();
                ; // + " " + name + "[" + elementCount + "];";
            case Kind::Int:
            case Kind::Uint:
            case Kind::Float:
            case Kind::Boolean:
                return name + ";";
            default:
                return "";
            }
        }
    };

    static TL::Ref<Type> i8      = Type::Create("i8", Type::Kind::Int, sizeof(int8_t), alignof(int8_t));
    static TL::Ref<Type> i16     = Type::Create("i16", Type::Kind::Int, sizeof(int16_t), alignof(int16_t));
    static TL::Ref<Type> i32     = Type::Create("i32", Type::Kind::Int, sizeof(int32_t), alignof(int32_t));
    static TL::Ref<Type> i64     = Type::Create("i64", Type::Kind::Int, sizeof(int64_t), alignof(int64_t));
    static TL::Ref<Type> u8      = Type::Create("u8", Type::Kind::Uint, sizeof(uint8_t), alignof(uint8_t));
    static TL::Ref<Type> u16     = Type::Create("u16", Type::Kind::Uint, sizeof(uint16_t), alignof(uint16_t));
    static TL::Ref<Type> u32     = Type::Create("u32", Type::Kind::Uint, sizeof(uint32_t), alignof(uint32_t));
    static TL::Ref<Type> u64     = Type::Create("u64", Type::Kind::Uint, sizeof(uint64_t), alignof(uint64_t));
    static TL::Ref<Type> f32     = Type::Create("f32", Type::Kind::Float, sizeof(float), alignof(float));
    static TL::Ref<Type> f64     = Type::Create("f64", Type::Kind::Float, sizeof(double), alignof(double));
    static TL::Ref<Type> boolean = Type::Create("bool", Type::Kind::Boolean, sizeof(bool), alignof(bool));
    // Vector types
    static TL::Ref<Type> vec2    = Type::CreateVector("vec2", f32, 2);
    static TL::Ref<Type> vec3    = Type::CreateVector("vec3", f32, 3);
    static TL::Ref<Type> vec4    = Type::CreateVector("vec4", f32, 4);
    //
    static TL::Ref<Type> ivec2   = Type::CreateVector("ivec2", i32, 2);
    static TL::Ref<Type> ivec3   = Type::CreateVector("ivec3", i32, 3);
    static TL::Ref<Type> ivec4   = Type::CreateVector("ivec4", i32, 4);
    //
    static TL::Ref<Type> uvec2   = Type::CreateVector("uvec2", u32, 2);
    static TL::Ref<Type> uvec3   = Type::CreateVector("uvec3", u32, 3);
    static TL::Ref<Type> uvec4   = Type::CreateVector("uvec4", u32, 4);
    //
    static TL::Ref<Type> bvec2   = Type::CreateVector("bvec2", boolean, 2);
    static TL::Ref<Type> bvec3   = Type::CreateVector("bvec3", boolean, 3);
    static TL::Ref<Type> bvec4   = Type::CreateVector("bvec4", boolean, 4);
    // Matrix types (float only)
    static TL::Ref<Type> mat2    = Type::CreateMatrix("mat2", f32, 2, 2);
    static TL::Ref<Type> mat3    = Type::CreateMatrix("mat3", f32, 3, 3);
    static TL::Ref<Type> mat4    = Type::CreateMatrix("mat4", f32, 4, 4);
    //
    static TL::Ref<Type> mat2x3  = Type::CreateMatrix("mat2x3", f32, 2, 3);
    static TL::Ref<Type> mat3x2  = Type::CreateMatrix("mat3x2", f32, 3, 2);
    static TL::Ref<Type> mat2x4  = Type::CreateMatrix("mat2x4", f32, 2, 4);
    static TL::Ref<Type> mat4x2  = Type::CreateMatrix("mat4x2", f32, 4, 2);
    static TL::Ref<Type> mat3x4  = Type::CreateMatrix("mat3x4", f32, 3, 4);
    static TL::Ref<Type> mat4x3  = Type::CreateMatrix("mat4x3", f32, 4, 3);
}; // namespace CPPStructBuilder

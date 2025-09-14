#pragma once

#include <string_view>
#include <string>

#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <iostream>

namespace codegen
{
    class Generator;
    class Scope;
    class Type;
    class Variable;
    class Function;

    class Generator
    {
    public:
        Generator();

        Scope* rootScope() { return m_root.get(); }

        // import an external type (e.g. float, int, ...)
        Type* importType(std::string_view name, Scope* scope);

    private:
        std::unique_ptr<Scope> m_root;

    public:
        Type* const t_float;
        Type* const t_int;
        Type* const t_uint;
        Type* const t_bool;

        Type* const t_float2;
        Type* const t_float3;
        Type* const t_float4;

        Type* const t_int2;
        Type* const t_int3;
        Type* const t_int4;

        Type* const t_uint2;
        Type* const t_uint3;
        Type* const t_uint4;

        Type* const t_float2x2;
        Type* const t_float3x3;
        Type* const t_float4x4;

        Type* const t_void;
    };

    class Declaration
    {
    public:
        enum class Kind
        {
            Type,
            Variable,
            Function
        };

        Declaration(std::string name, Scope* scope, Kind kind)
            : m_name(std::move(name))
            , m_scope(scope)
            , m_kind(kind)
        {
        }

        std::string getName() const { return m_name; }

        Scope* getScope() const { return m_scope; }

        Kind getKind() const { return m_kind; }

    private:
        std::string m_name;
        Scope*      m_scope;
        Kind        m_kind;
    };

    class Variable : public Declaration
    {
    public:
        Variable(std::string name, Type* type, Scope* scope, bool static_constexpr, std::string_view value)
            : Declaration(std::move(name), scope, Kind::Variable)
            , m_type(type)
            , m_isStatic(static_constexpr)
            , m_isConstexpr(static_constexpr)
            , m_value(value)
        {
        }

        Type* getType() const { return m_type; }

        void render(std::ostream& os, uint32_t indentation) const;

    private:
        Type*       m_type;
        bool        m_isStatic;
        bool        m_isConstexpr;
        std::string m_value;
    };

    class Function : public Declaration
    {
    public:
        Function(std::string name, Type* returnType, Scope* scope, bool isStatic);

        Function* addArgument(std::string name, Type* type);
        Function* addLine(std::string line);

        void render(std::ostream& os, uint32_t indentation) const;

    private:
        Type*                                      m_returnType;
        std::vector<std::pair<std::string, Type*>> m_arguments;
        std::vector<std::string>                   m_body;
        bool m_isStatic = false;
    };

    enum class ScopeKind
    {
        Namespace,
        Struct,
        Root
    };

    class Scope
    {
    public:
        Scope()
            : m_name("")
            , m_parent(nullptr)
            , m_kind(ScopeKind::Namespace)
        {
        }

        Scope(std::string name, Scope* parent, ScopeKind kind)
            : m_name(std::move(name))
            , m_parent(parent)
            , m_kind(kind)
        {
        }

        const std::string& getName() const { return m_name; }

        Type*     createType(std::string_view name, bool imported = false);
        Variable* createVariable(std::string_view name, Type* type, bool static_constexpr = false, std::string_view value = {});
        Scope*    createNamespace(std::string_view name);
        Function* createFunction(std::string_view name, Type* returnType, bool isStatic = false);

        void render(std::ostream& os, uint32_t indentation) const;

    private:
        friend class Generator;
        friend class Type;

        ScopeKind   m_kind;
        std::string m_name;
        Scope*      m_parent;

        // lookup + ownership
        std::unordered_map<std::string, std::unique_ptr<Scope>>    m_namespaces;
        std::unordered_map<std::string, std::unique_ptr<Type>>     m_types;
        std::unordered_map<std::string, std::unique_ptr<Variable>> m_variables;
        std::unordered_map<std::string, std::unique_ptr<Function>> m_functions;

        // order preservation
        std::vector<Scope*>    m_namespaceOrder;
        std::vector<Type*>     m_typeOrder;
        std::vector<Variable*> m_variableOrder;
        std::vector<Function*> m_functionOrder;
    };

    class Type : public Declaration
    {
    public:
        Type(std::string name, Scope* scope, bool imported = false)
            : Declaration(std::move(name), scope, Kind::Type)
            , m_isImported(imported)
            , m_innerScope(std::make_unique<Scope>(this->getName(), scope, ScopeKind::Struct))
        {
        }

        Variable* addField(std::string_view name, Type* type, bool static_constexpr = false, std::string_view value = {});
        Function* addFunction(std::string_view name, Type* returnType, bool isStatic = false);

        Scope* innerScope();

        void render(std::ostream& os, uint32_t indentation) const;

    private:
        bool                   m_isImported;
        std::unique_ptr<Scope> m_innerScope;
    };
}; // namespace codegen

#include "codegen.h"

#include <TL/Compiler.hpp>

namespace codegen
{
    Generator::Generator()
        : m_root(std::make_unique<Scope>())
        , t_float(importType("float", m_root.get()))
        , t_int(importType("int", m_root.get()))
        , t_uint(importType("uint", m_root.get()))
        , t_bool(importType("bool", m_root.get()))
        , t_float2(importType("float2", m_root.get()))
        , t_float3(importType("float3", m_root.get()))
        , t_float4(importType("float4", m_root.get()))
        , t_int2(importType("int2", m_root.get()))
        , t_int3(importType("int3", m_root.get()))
        , t_int4(importType("int4", m_root.get()))
        , t_uint2(importType("uint2", m_root.get()))
        , t_uint3(importType("uint3", m_root.get()))
        , t_uint4(importType("uint4", m_root.get()))
        , t_float2x2(importType("float2x2", m_root.get()))
        , t_float3x3(importType("float3x3", m_root.get()))
        , t_float4x4(importType("float4x4", m_root.get()))
        , t_void(importType("void", m_root.get()))
    {
    }

    Type* Generator::importType(std::string_view name, Scope* scope)
    {
        auto  type = std::make_unique<Type>(std::string(name), scope, true);
        Type* ptr  = type.get();
        scope->m_types.emplace(ptr->getName(), std::move(type));
        scope->m_typeOrder.push_back(ptr);
        return ptr;
    }

    void Variable::render(std::ostream& os, uint32_t indentation) const
    {
        std::string indent(indentation, ' ');

        if (m_isConstexpr)
            os << indent << "constexpr static ";
        else if (m_isStatic)
            os << indent << "static ";
        else
            os << indent;

        os << m_type->getName() << " " << getName();

        if (!m_value.empty())
            os << " = " << m_value;

        os << ";\n";
    }

    Function::Function(std::string name, Type* returnType, Scope* scope, bool isStatic)
        : Declaration(std::move(name), scope, Kind::Function)
        , m_returnType(returnType)
        , m_isStatic(isStatic)
    {
    }

    Function* Function::addArgument(std::string name, Type* type)
    {
        m_arguments.emplace_back(std::move(name), type);
        return this;
    }

    Function* Function::addLine(std::string line)
    {
        m_body.push_back(line);
        return this;
    }

    void Function::render(std::ostream& os, uint32_t indentation) const
    {
        std::string indent(indentation, ' ');
        os << "\n";

        os << indent;

        if (m_isStatic)
            os << "static ";

        os << m_returnType->getName() << " " << getName() << "(";
        for (size_t i = 0; i < m_arguments.size(); ++i)
        {
            os << m_arguments[i].second->getName() << " " << m_arguments[i].first;
            if (i + 1 < m_arguments.size())
                os << ", ";
        }
        os << ")\n"
           << indent << "{\n";
        for (auto& line : m_body)
            os << indent << "    " << line << "\n";
        os << indent << "}\n";
    }

    Type* Scope::createType(std::string_view name, bool imported)
    {
        auto  type = std::make_unique<Type>(std::string(name), this, imported);
        Type* ptr  = type.get();
        m_types.emplace(ptr->getName(), std::move(type));
        m_typeOrder.push_back(ptr);
        return ptr;
    }

    Variable* Scope::createVariable(std::string_view name, Type* type, bool static_constexpr, std::string_view value)
    {
        if (auto it = m_variables.find(name.data()); it != m_variables.end())
        {
            return it->second.get();
        }
        else
        {
            auto var = std::make_unique<Variable>(std::string(name), type, this, static_constexpr, value);
            auto ptr = m_variables.emplace(name, std::move(var)).first;
            m_variableOrder.push_back(ptr->second.get());
            return ptr->second.get();
        }
    }

    Scope* Scope::createNamespace(std::string_view name)
    {
        auto   ns  = std::make_unique<Scope>(std::string(name), this, ScopeKind::Namespace);
        Scope* ptr = ns.get();
        m_namespaces.emplace(ptr->m_name, std::move(ns));
        m_namespaceOrder.push_back(ptr);
        return ptr;
    }

    Function* Scope::createFunction(std::string_view name, Type* returnType, bool isStatic)
    {
        auto      fn  = std::make_unique<Function>(std::string(name), returnType, this, isStatic);
        Function* ptr = fn.get();
        m_functions.emplace(ptr->getName(), std::move(fn));
        m_functionOrder.push_back(ptr);
        return ptr;
    }

    void Scope::render(std::ostream& os, uint32_t indentation) const
    {
        std::string indent(indentation, ' ');

        uint32_t innerIndentation = m_parent ? 4 : 0;
        if (m_kind == ScopeKind::Namespace && m_parent != nullptr)
        {
            os << indent << "namespace " << m_name << "\n";
            os << indent << "{\n";
        }

        for (auto* ns : m_namespaceOrder)
            ns->render(os, indentation + innerIndentation);

        for (auto* t : m_typeOrder)
            t->render(os, indentation + innerIndentation);

        for (auto* v : m_variableOrder)
            v->render(os, indentation + innerIndentation);

        for (auto* f : m_functionOrder)
            f->render(os, indentation + innerIndentation);

        if (m_kind == ScopeKind::Namespace && m_parent != nullptr)
            os << indent << "}\n";
    }

    Variable* Type::addField(std::string_view name, Type* type, bool static_constexpr, std::string_view value)
    {
        // TL_ASSERT(name.empty() == false);
        return m_innerScope->createVariable(name, type, static_constexpr, value);
    }

    Function* Type::addFunction(std::string_view name, Type* returnType, bool isStatic)
    {
        return m_innerScope->createFunction(name, returnType, isStatic);
    }

    Scope* Type::innerScope()
    {
        return m_innerScope.get();
    }

    void Type::render(std::ostream& os, uint32_t indentation) const
    {
        if (m_isImported)
            return;

        std::string indent(indentation, ' ');
        os << indent << "struct " << getName() << "\n";
        os << indent << "{\n";

        for (auto* t : m_innerScope->m_typeOrder)
        {
            t->render(os, indentation + 4);
            os << "\n";
        }

        for (auto* var : m_innerScope->m_variableOrder)
            var->render(os, indentation + 4);

        for (auto* f : m_innerScope->m_functionOrder)
            f->render(os, indentation + 4);

        os << indent << "};\n\n";
    }

} // namespace codegen

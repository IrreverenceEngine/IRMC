#pragma once

#include <IRMC_CTypes.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Log.hpp>

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <map>

namespace IRMC {

    class ArgParser;

    using ArgValue = std::variant<Float64, std::string>;

    class ArgVariable {
    public:
        enum Action : UInt8 {
            ACTION_UNKNOWN,
            ACTION_SET,
            ACTION_ADD
        };

        enum Type : UInt8 {
            TYPE_UNKNOWN,
            TYPE_FLAG,
            TYPE_NUMBER,
            TYPE_PATH,
            TYPE_STRING
        };

        ArgVariable() {}
        ArgVariable(Action action, Type type, const char* name, const char* sname = nullptr, const char* desc = nullptr);

        void Perform(const ArgValue& val);
        void PerformSet(const ArgValue& val);
        void PerformAdd(const ArgValue& val);

        Action GetAction() const IRMC_RETURN(m_Action)
        Type GetType() const IRMC_RETURN(m_Type)
        const char* GetName() const IRMC_RETURN(m_Name)
        const char* GetShort() const IRMC_RETURN(m_Short)
        const char* GetDesc() const IRMC_RETURN(m_Desc)
        const std::vector<ArgValue>& GetList() const IRMC_RETURN(m_List)

        bool AsBool() const IRMC_RETURN((bool)std::get<Float64>(m_Val))
        Float64 AsNumber() const IRMC_RETURN(std::get<Float64>(m_Val))
        std::string AsString() const IRMC_RETURN(std::get<std::string>(m_Val))

    private:
        Action m_Action = ACTION_UNKNOWN;
        Type m_Type = TYPE_UNKNOWN;

        const char* m_Name = nullptr;
        const char* m_Short = nullptr;
        const char* m_Desc = nullptr;

        ArgValue m_Val;
        std::vector<ArgValue> m_List;
    };

    class ArgParser {
    public:
        ArgParser(const std::vector<ArgVariable>& vars, int argc, char** argv);
    
        const ArgVariable& GetVariable(const char* name);
        void PrintHelp();
        
    private:
        struct Token {
            enum Kind : UInt8 {
                KIND_STRING,
                KIND_NUMBER,
                KIND_NAMESIG,
                KIND_SNAMESIG,
                KIND_COMMA
            };

            Kind kind;
            ArgValue data;
        };

        void Tokenize(const char* args);
        void Parse();

        const Token& TknPeek() const IRMC_RETURN(m_Tokens[m_TokenPos])
        const Token& TknAdvance() IRMC_RETURN(m_Tokens[m_TokenPos++])
        bool TknIsEnd() const IRMC_RETURN(m_TokenPos >= m_Tokens.size())

    private:
        std::map<std::string_view, ArgVariable> m_ArgValues;
        std::map<std::string_view, std::string_view> m_CharName;
        std::vector<Token> m_Tokens;
        UInt64 m_TokenPos = 0;

        std::string m_HelpMsg;
    };
}

#include <IRMC_Macro.hpp>
#include <IRMC_Args.hpp>

#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>
#include <ctype.h>

namespace IRMC {

    std::string SanitizePath(std::string_view inputPath)
    {
        if (inputPath.empty()) {
            return "";
        }

        std::filesystem::path p(inputPath);

        while (!p.empty() && p.has_filename() == false) {
            p = p.parent_path();
        }

        p = std::filesystem::absolute(p).lexically_normal();

        return p;
    }  

    ArgVariable::ArgVariable(Action action, Type type, const char* name, const char* character, const char* description) :
        m_Action(action), m_Type(type), m_Name(name), m_Short(character), m_Desc(description)
    {
        switch (type) {
        case TYPE_FLAG: m_Action = ACTION_SET;
        case TYPE_NUMBER:
        {
            m_Val = 0.0;
            break;
        }
        case TYPE_PATH:
        case TYPE_STRING:
        {
            m_Val = "";
            break;
        }
        default: IRMC_MSG(FATAL, "Please set type of Argument Variable");
        }
    }

    void ArgVariable::Perform(const ArgValue& val)
    {        
        switch (m_Action) {
        case ACTION_SET:
        {
            PerformSet(val);
            break;
        }
        case ACTION_ADD:
        {
            PerformAdd(val);
            break;
        }
        default: break;
        }
    }
    
    void ArgVariable::PerformSet(const ArgValue& val)
    {
        switch (m_Type) {
        case TYPE_FLAG:
        {
            m_Val = 1.0;
            break;
        }
        case TYPE_NUMBER:
        case TYPE_STRING:
        case TYPE_PATH:
        {
            m_Val = (m_Type == TYPE_PATH) ? SanitizePath(std::get<std::string>(val)) : val;
            break;
        }
        default: break;
        }
    }
    
    void ArgVariable::PerformAdd(const ArgValue& val)
    {
        m_List.emplace_back((m_Type == TYPE_PATH) ? SanitizePath(std::get<std::string>(val)) : val);
    }

    ArgParser::ArgParser(const std::vector<ArgVariable>& vars, int argc, char** argv)
    {
        m_HelpMsg = "Usage:\n";

        for (const ArgVariable& var : vars) {
            m_ArgValues[var.GetName()] = var;
            if (var.GetShort()) {
                m_CharName[var.GetShort()] = var.GetName();
            }

            m_HelpMsg += "    \"";
            m_HelpMsg += var.GetName();
            m_HelpMsg += "\"";

            if (var.GetShort()) {
                m_HelpMsg += " or \"";
                m_HelpMsg += var.GetShort();
                m_HelpMsg += "\"";
            }

            if (var.GetDesc()) {
                m_HelpMsg += ": ";
                m_HelpMsg += var.GetDesc();
            }

            m_HelpMsg += '\n';
        }

        std::string args;

        for (Int32 i = 1; i < argc; i++) {
            if (i > 1)
                args += ' ';

            args += argv[i];
        }

        Tokenize(args.c_str());
        Parse();
    }

    const ArgVariable& ArgParser::GetVariable(const char* name)
    {
        ArgVariable* argval = nullptr;
        std::string namestr = name;

        auto it_flag = m_CharName.find(namestr.c_str());
        if (it_flag != m_CharName.end()) {
            namestr = it_flag->second;
        }

        auto it_argval = m_ArgValues.find(namestr.c_str());
        if (it_argval != m_ArgValues.end()) {
            argval = &it_argval->second;
        } else {
            IRMC_MSG(FATAL, "Argument variable called %s doesn't exist", name);
        }

        return *argval;
    }
    
    void ArgParser::PrintHelp()
    {
        IRMC_MSG(INFO, "%s", m_HelpMsg.c_str());
    }
    
    void ArgParser::Tokenize(const char* args)
    {
        const char* argp = args;
        while (char c = *argp++) {
            switch (c) {
                case ' ': continue;
                case '=': continue;
                case ',': continue;
                case '-': {
                    if (*argp == '-') {
                        m_Tokens.push_back({ Token::KIND_NAMESIG });
                        argp++;
                    } else if (!isdigit(*argp)) {
                        m_Tokens.push_back({ Token::KIND_SNAMESIG });
                    }

                    break;
                }
                case '"': {
                    const char* begin = argp;

                    while ((c = *argp)) {
                        if (c == '"') {
                            break;
                        }

                        argp++;
                    }

                    const char* end = argp++;
                    size_t size = end - begin;

                    Token token;
                    token.kind = Token::KIND_STRING;
                    token.data = (std::string) { begin, size };

                    m_Tokens.emplace_back(token);

                    break;
                }
            }

            if (isalpha(c) || c == '_' || c == '/') {
                const char* begin = argp - 1;

                while ((c = *argp)) {
                    if (c == ' ' || c == ',') {
                        break;
                    }

                    argp++;
                }

                const char* end = argp;
                size_t size = end - begin;

                Token token;
                token.kind = Token::KIND_STRING;
                token.data = (std::string) { begin, size };

                m_Tokens.emplace_back(token);

            } else if (isdigit(c) || (c == '-' && isdigit(*argp)) || c == '.') {
                Token token;
                token.kind = Token::KIND_NUMBER;
                token.data = strtod(argp - 1, (char**)&argp);

                m_Tokens.emplace_back(token);
            }
        }
    }

    void ArgParser::Parse()
    {
        Token tkn;
        Token argname;
        ArgVariable* argval;
        std::string argname_str;
        UInt8 valcount;

        while (!TknIsEnd()) {
            tkn = TknAdvance();
            if (tkn.kind != Token::KIND_NAMESIG && tkn.kind != Token::KIND_SNAMESIG) {
                IRMC_MSG(FATAL, "Expected argument signifier '--' or '-'");
            }

            argname = TknAdvance();
            if (argname.kind != Token::KIND_STRING) {
                IRMC_MSG(FATAL, "Argument name must be a string");
            }

            argname_str = std::get<std::string>(argname.data);

            if (tkn.kind == Token::KIND_SNAMESIG) {
                auto it_flag = m_CharName.find(argname_str.c_str());
                if (it_flag != m_CharName.end()) {
                    argname_str = it_flag->second;
                }            
            }

            auto it_argval = m_ArgValues.find(argname_str.c_str());
            if (it_argval != m_ArgValues.end()) {
                argval = &it_argval->second;
            } else {
                IRMC_MSG(FATAL, "There is no argument called \"%s\"", argname_str.c_str());
            }

            if (argval->GetType() == ArgVariable::TYPE_FLAG) {
                argval->PerformSet({});
                continue;
            }

            valcount = 0;

            while (!TknIsEnd() && TknPeek().kind != Token::KIND_NAMESIG && TknPeek().kind != Token::KIND_SNAMESIG) {
                if (argval->GetAction() == ArgVariable::ACTION_SET && valcount == 1) {
                    break;
                }
                
                tkn = TknAdvance();
                argval->Perform(tkn.data);

                valcount++;
            }
        }
    }

    
}

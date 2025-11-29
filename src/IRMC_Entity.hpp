#pragma once

#include <IRMC_Macro.hpp>

#include <map>
#include <vector>
#include <string>

namespace IRMC {
    class Brush;
    class Plane;

    class Entity {
    public:
        void SetKeyValue(const std::string& key, const std::string& val);
        const std::string& GetKeyValue(const std::string& key) const IRMC_RETURN(m_KeyVals.at(key))

        void AddBrush(const Brush& brush);

        std::map<std::string, std::string>& GetKeyValues() IRMC_RETURN(m_KeyVals)
        const std::map<std::string, std::string>& GetKeyValues() const IRMC_RETURN(m_KeyVals)
        std::vector<Brush>& GetBrushes() IRMC_RETURN(m_Brushes)
        const std::vector<Brush>& GetBrushes() const IRMC_RETURN(m_Brushes)

    private:
        std::map<std::string, std::string> m_KeyVals;
        std::vector<Brush> m_Brushes;
    };
}

#include <IRMC_Entity.hpp>
#include <IRMC_Brush.hpp>

namespace IRMC {

    void Entity::SetKeyValue(const std::string& key, const std::string& val)
    {
        m_KeyVals[key] = val;
    }

    void Entity::AddBrush(const Brush& brush)
    {
        m_Brushes.push_back(brush);
    }

}
#include "Common.h"

// SEARCH PARAMETERS
namespace SparCraft
{
    namespace System
    {
        void FatalError(const std::string & errorMessage)
        {
            std::cerr << "\n\n\nSparCraft Fatal Error: \n\n\n      " << errorMessage << "\n\n";
            throw(SPARCRAFT_FATAL_ERROR);
        }
        
        void checkSupportedUnitType(const BWAPI::UnitType & type)
        {
            if (type == BWAPI::UnitTypes::None || type == BWAPI::UnitTypes::Unknown)
            {
                System::FatalError("Unknown unit type in experiment file");
            }

            if (type == BWAPI::UnitTypes::Protoss_Corsair || 
                type == BWAPI::UnitTypes::Zerg_Devourer || 
                type == BWAPI::UnitTypes::Zerg_Scourge ||
                type == BWAPI::UnitTypes::Terran_Valkyrie)
            {
                System::FatalError("Units with just air weapons currently not supported correctly: " + type.getName());
            }

            if (type.isBuilding() && (type != BWAPI::UnitTypes::Protoss_Photon_Cannon || type != BWAPI::UnitTypes::Zerg_Sunken_Colony || type != BWAPI::UnitTypes::Terran_Missile_Turret))
            {
                System::FatalError("Non-attacking buildings not currently supported: " + type.getName());
            }

            if (type.isSpellcaster())
            {
                System::FatalError("Spell casting units not currently supported: " + type.getName());
            }
        }
    }
};

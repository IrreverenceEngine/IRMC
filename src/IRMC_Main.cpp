#include <IRX_Common.hpp>
#include <IRX_Args.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Plane.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_Game.hpp>
#include <IRMC_Octree.hpp>

#include <filesystem>

int main(int argc, char** argv)
{
    if (argc <= 1) {
        IRX_MSG(FATAL, "Missing Arguments!");
    }

    IRX::ArgParser parser({
        { IRX::ArgVariable::TYPE_PATH, "file", "f", "File to compile" },
        { IRX::ArgVariable::TYPE_PATH, "output", "o", "Output compiled map to directory" },
        { IRX::ArgVariable::TYPE_PATH, "game", "g", "Path to the game's directory | Example: Some/Dir/Irreverence" },
        { IRX::ArgVariable::TYPE_FLAG, "no-navmesh", "N", "Disable the Navmesh stage" },
        { IRX::ArgVariable::TYPE_FLAG, "no-lightmap", "L", "Disable the Lightmap stage" },
        { IRX::ArgVariable::TYPE_FLAG, "compress", "z", "Compress the file" },
        { IRX::ArgVariable::TYPE_FLAG, "colored", "c", "Enable colored output" },
        { IRX::ArgVariable::TYPE_FLAG, "help", "h", "Shows this help message :)" }
    }, argc, argv);

    if (!parser.GetVariable("colored").AsBool()) IRX::Log::ColorOutput(false);
    
    if (parser.GetVariable("help").AsBool()) {
        parser.PrintHelp();
        return 0;
    }
    
    std::string file = parser.GetVariable("file").AsString();
    std::string output = parser.GetVariable("output").AsString();
    std::string game = parser.GetVariable("game").AsString();
    std::filesystem::path filepath = file;

    if (file.empty()) IRX_MSG(FATAL, "Missing file path");
    else if (filepath.extension() != ".map") IRX_MSG(FATAL, "File extension has to be .map");

    if (output.empty()) output = filepath.parent_path().string();
    if (output.empty()) IRX_MSG(FATAL, "Missing game path");

    IRMC::Game::Init("Irreverence", game.c_str());

    IRMC::Map map;
    if (!parser.GetVariable("no-navmesh").AsBool()) map.EnableStage(IRMC::Stage::LEVEL_NAVMESH);
    if (!parser.GetVariable("no-lightmap").AsBool()) map.EnableStage(IRMC::Stage::LEVEL_LIGHTMAP);

    map.LoadMapFromFile(file.c_str());
    map.CompileMap((output + "/" + filepath.stem().string() + ".irbm").c_str(), parser.GetVariable("compress").AsBool());

    return 0;
}

#include <IRMC_Brush.hpp>
#include <IRMC_Plane.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_CTypes.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_Game.hpp>
#include <IRMC_Args.hpp>
#include <filesystem>

int main(int argc, char** argv)
{
    if (argc <= 1) {
        IRMC_MSG(FATAL, "Missing Arguments!");
    }

    IRMC::ArgParser parser({
        { IRMC::ArgVariable::TYPE_PATH, "file", "f", "File to compile" },
        { IRMC::ArgVariable::TYPE_PATH, "output", "o", "Output compiled map to directory" },
        { IRMC::ArgVariable::TYPE_PATH, "game", "g", "Path to the game's directory | Example: Some/Dir/Irreverence" },
        { IRMC::ArgVariable::TYPE_FLAG, "--no-navmesh", "N", "Disable the Navmesh stage" },
        { IRMC::ArgVariable::TYPE_FLAG, "--no-lightmap", "L", "Disable the Lightmap stage" },
        { IRMC::ArgVariable::TYPE_FLAG, "colored", "c", "Enable colored output" },
        { IRMC::ArgVariable::TYPE_FLAG, "help", "h", "Shows this help message :)" }
    }, argc, argv);

    IRMC::Log::ColorOutput(parser.GetVariable("colored").AsBool());
    if (parser.GetVariable("help").AsBool()) parser.PrintHelp();
    
    std::string file = parser.GetVariable("file").AsString();
    std::string output = parser.GetVariable("output").AsString();
    std::string game = parser.GetVariable("game").AsString();
    std::filesystem::path filepath = file;

    if (file.empty()) IRMC_MSG(FATAL, "Missing file path");
    else if (filepath.extension() != ".map") IRMC_MSG(FATAL, "File extension has to be .map");

    if (output.empty()) output = filepath.parent_path().string();
    if (output.empty()) IRMC_MSG(FATAL, "Missing game path");

    IRMC::Game::Init("Irreverence", game.c_str());

    IRMC::Map map;
    if (!parser.GetVariable("--no-navmesh").AsBool()) map.EnableStage(IRMC::Stage::LEVEL_NAVMESH);
    if (!parser.GetVariable("--no-lightmap").AsBool()) map.EnableStage(IRMC::Stage::LEVEL_LIGHTMAP);

    map.LoadMapFromFile(file.c_str());
    map.CompileMap((output + "/" + filepath.stem().string() + ".irbm").c_str());

    return 0;
}

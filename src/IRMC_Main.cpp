#include <IRMC_Brush.hpp>
#include <IRMC_Plane.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_CTypes.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_Game.hpp>

#include <cstring>

#include <cstdlib>
#include <cstdio>
#include <filesystem>

std::filesystem::path SanitizePath(const std::filesystem::path& inputPath) {
    std::filesystem::path absolute = std::filesystem::absolute(inputPath);
    absolute = absolute.lexically_normal();

    if (absolute != absolute.root_path() && absolute.has_filename() && absolute.filename() == ".") {
        absolute = absolute.parent_path();
    }

    return absolute;
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        IRMC_MSG(FATAL, "Missing Arguments!");
    }

    std::filesystem::path filePath;
    std::filesystem::path outputPath;
    std::filesystem::path gamePath;

    for (IRMC::UInt32 i = 1; i < argc; i++) {
        std::string_view str = argv[i];

        if (str == "-f" || str == "--file") {
            if (!filePath.empty()) {
                IRMC_MSG(FATAL, "-f or --file is already specified");
            }

            char* subArg = argv[++i];
            if (!subArg) {
                IRMC_MSG(FATAL, "Did not specify path when using -f or --file");
            }

            filePath = SanitizePath(subArg);
        } else if (str == "-o" || str == "--output") {
            if (!outputPath.empty()) {
                IRMC_MSG(FATAL, "-o or --output is already specified");
            }

            char* subArg = argv[++i];
            if (!subArg) {
                IRMC_MSG(FATAL, "Did not specify path when using -o or --output");
            }

            outputPath = SanitizePath(subArg);
        } else if (str == "-g" || str == "--game") {
            if (!gamePath.empty()) {
                IRMC_MSG(FATAL, "-g or --game is already specified");
            }

            char* subArg = argv[++i];
            if (!subArg) {
                IRMC_MSG(FATAL, "Did not specify path when using -g or --game");
            }

            gamePath = SanitizePath(subArg);
        } else if (str == "-h" || str == "--help") {
            IRMC_MSG(INFO, "Usage:\n"
                "    \"-f\" or \"--file\": File to compile\n"
                "    \"-o\" or \"--output\": Output compiled map to directory\n"
                "    \"-g\" or \"--game\": Path to the game's directory, normally one directory behind assets. Example: some/dir/Irreverence\n"
                "    \"-h\" or \"--help\": Shows this help message :D\n");
        } else {
            filePath = SanitizePath(str);
        }
    }

    if (!filePath.empty()) {
        if (filePath.extension() != ".map") {
            IRMC_MSG(FATAL, "File extension has to be .map");
        }
    } else {
        IRMC_MSG(FATAL, "Missing file path");
    }

    if (outputPath.empty()) {
        outputPath = SanitizePath(filePath.parent_path().string() + "/");
    }

    if (gamePath.empty()) {
        IRMC_MSG(FATAL, "Missing game path");
    }

    IRMC::Game::Init("Irreverence", gamePath.c_str());

    IRMC::Map myMap;
    myMap.LoadMapFromFile(filePath.c_str());
    myMap.CompileMap((outputPath.string() + filePath.stem().string() + ".irbm").c_str());

    return 0;
}
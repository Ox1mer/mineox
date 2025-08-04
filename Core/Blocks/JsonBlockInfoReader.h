#pragma once

#include <json/json.hpp>

#include "Blocks.h"
#include "PathProvider.h"

#include "BlockModelStructs.h"
#include "BlockInfoStruct.h"

#include "Logger.h"

using json = nlohmann::json;

class JsonBlockInfoReader {
public:
    std::optional<BlockModel> readBlockModelInfo(Blocks block) {
        auto blockString = toString(block);
        if (blockString == "air") {
            return std::nullopt;
        }
        std::ifstream f(PathProvider::getInstance().getBlocksJsonFolderPath().string() + "/" + blockString + ".json");

        Logger::getInstance().Log("Loading file: " + PathProvider::getInstance().getBlocksJsonFolderPath().string() + "/" + blockString + ".json");

        json j;
        f >> j;

        BlockModel model;
        model.credit = j["credit"];
        model.textures = j["textures"].get<std::map<std::string, std::string>>();

        for (const auto& elem : j["elements"]) {
            BlockModelElement e;
            e.from = elem["from"].get<std::vector<float>>();
            e.to = elem["to"].get<std::vector<float>>();

            auto rot = elem["rotation"];
            e.rotation.angle = rot["angle"];
            e.rotation.axis = rot["axis"];
            e.rotation.origin = rot["origin"].get<std::vector<float>>();

            auto faces = elem["faces"];
            for (auto it = faces.begin(); it != faces.end(); ++it) {
                BlockModelFace f;
                f.uv = it.value()["uv"].get<std::vector<float>>();
                f.texture = it.value()["texture"];
                e.faces[it.key()] = f;
            }

            model.elements.push_back(e);
        }

        for (const auto& [key, tex] : model.textures) {
            Logger::getInstance().Log("Texture " + key + ": " + tex);
        }

        for (const auto& e : model.elements) {
            std::string fromStr = "Element from: ";
            for (auto v : e.from) {
                fromStr += std::to_string(v) + " ";
            }
            Logger::getInstance().Log(fromStr);
        }

        return model;
    }

    std::optional<BlockInfo> readBlockInfo(Blocks block) {
        auto blockString = toString(block);
        if (blockString == "air") {
            return std::nullopt;
        }
        auto filePath = PathProvider::getInstance().getBlocksConfigsFolder().string() + "/" + blockString + ".json";

        std::ifstream f(filePath);
        if (!f.is_open()) {
            std::cerr << "Cannot open file: " << filePath << std::endl;
            return std::nullopt;
        }

        json j;
        try {
            f >> j;
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error in " << filePath << ": " << e.what() << std::endl;
            return std::nullopt;
        }

        BlockInfo info;

        if (j.contains("name")) info.name = j["name"];
        if (j.contains("isTransparent")) info.isTransparent = j["isTransparent"];
        if (j.contains("isSolid")) info.isSolid = j["isSolid"];
        if (j.contains("isOpaque")) info.isOpaque = j["isOpaque"];

        return info;
    }

};
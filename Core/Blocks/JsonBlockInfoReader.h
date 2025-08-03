#pragma once

#include <json/json.hpp>

#include "Blocks.h"
#include "PathProvider.h"

using json = nlohmann::json;

struct Rotation {
    float angle;
    std::string axis;
    std::vector<float> origin;
};

struct Face {
    std::vector<float> uv;
    std::string texture;
};

struct Element {
    std::vector<float> from;
    std::vector<float> to;
    Rotation rotation;
    std::map<std::string, Face> faces;
};

struct Model {
    std::string credit;
    std::map<std::string, std::string> textures;
    std::vector<Element> elements;
};

class JsonBlockInfoReader {
public:
    Model readBlockInfo(Blocks block) {
        auto blockString = toString(block);
        std::ifstream f(PathProvider::getInstance().getBlocksJsonFolderPath().string() + blockString + ".json");

        json j;
        f >> j;

        Model model;
        model.credit = j["credit"];
        model.textures = j["textures"].get<std::map<std::string, std::string>>();

        for (const auto& elem : j["elements"]) {
            Element e;
            e.from = elem["from"].get<std::vector<float>>();
            e.to = elem["to"].get<std::vector<float>>();

            auto rot = elem["rotation"];
            e.rotation.angle = rot["angle"];
            e.rotation.axis = rot["axis"];
            e.rotation.origin = rot["origin"].get<std::vector<float>>();

            auto faces = elem["faces"];
            for (auto it = faces.begin(); it != faces.end(); ++it) {
                Face f;
                f.uv = it.value()["uv"].get<std::vector<float>>();
                f.texture = it.value()["texture"];
                e.faces[it.key()] = f;
            }

            model.elements.push_back(e);
        }

        std::cout << "Credit: " << model.credit << "\n";

        for (const auto& [key, tex] : model.textures) {
            std::cout << "Texture " << key << ": " << tex << "\n";
        }

        for (const auto& e : model.elements) {
            std::cout << "Element from: ";
            for (auto v : e.from) std::cout << v << " ";
            std::cout << "\n";
        }

        return model;
    }


};
#pragma once

struct ChunkBlocksOpaqueData
{
    static constexpr int SIZE = 32;

    ChunkBlocksOpaqueData() 
        : data(SIZE*SIZE*SIZE, false) {}

    bool isOpaque(int x, int y, int z) const {
        assert(inBounds(x, y, z));
        return data[toIndex(x, y, z)];
    }

    void setOpaque(int x, int y, int z, bool opaque) {
        assert(inBounds(x, y, z));
        data[toIndex(x, y, z)] = opaque;
    } 

    std::string toDebugString() const {
        std::ostringstream oss;
        int count = 0;

        oss << "ChunkBlocksOpaqueData Debug:\n";

        for (int z = 0; z < SIZE; ++z) {
            for (int y = 0; y < SIZE; ++y) {
                for (int x = 0; x < SIZE; ++x) {
                    if (isOpaque(x, y, z)) {
                        ++count;
                        oss << "Opaque Block at (" 
                            << x << ", " << y << ", " << z << ")\n";
                    }
                }
            }
        }

        if (count == 0) {
            oss << "No opaque blocks found.\n";
        } else {
            oss << "Total opaque blocks: " << count << "\n";
        }

        return oss.str();
    }

private:
    std::vector<bool> data;

    static int toIndex(int x, int y, int z) {
        return x + SIZE * (y + SIZE * z);
    }

    static bool inBounds(int x, int y, int z) {
        return (x >= 0 && x < SIZE) && (y >= 0 && y < SIZE) && (z >= 0 && z < SIZE);
    }
};

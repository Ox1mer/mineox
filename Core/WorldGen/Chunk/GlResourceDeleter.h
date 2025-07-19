#pragma once

#include <vector>
#include <mutex>
#include <glad\glad.h>

class GLResourceDeleter {
public:

    static GLResourceDeleter& getInstance() {
        static GLResourceDeleter instance;
        return instance;
    }

    void queueDeleteVAO(unsigned int vao) {
        std::lock_guard<std::mutex> lock(mutex_);
        vaosToDelete.push_back(vao);
    }
    void queueDeleteVBO(unsigned int vbo) {
        std::lock_guard<std::mutex> lock(mutex_);
        vbosToDelete.push_back(vbo);
    }
    void queueDeleteEBO(unsigned int ebo) {
        std::lock_guard<std::mutex> lock(mutex_);
        ebosToDelete.push_back(ebo);
    }

    void processDeletes(size_t maxDeletesPerFrame = 1) {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto deleteUpTo = [&](std::vector<unsigned int>& queue, auto deleter) {
            size_t count = std::min(queue.size(), maxDeletesPerFrame);
            for (size_t i = 0; i < count; ++i) {
                deleter(queue[i]);
            }
            queue.erase(queue.begin(), queue.begin() + count);
        };

        deleteUpTo(vaosToDelete, [](unsigned int id) { glDeleteVertexArrays(1, &id); });
        deleteUpTo(vbosToDelete, [](unsigned int id) { glDeleteBuffers(1, &id); });
        deleteUpTo(ebosToDelete, [](unsigned int id) { glDeleteBuffers(1, &id); });
    }
}

private:

    GLResourceDeleter() = default;
    ~GLResourceDeleter() = default;
    GLResourceDeleter(const GLResourceDeleter&) = delete;
    GLResourceDeleter& operator=(const GLResourceDeleter&) = delete;

    std::mutex mutex_;
    std::vector<unsigned int> vaosToDelete;
    std::vector<unsigned int> vbosToDelete;
    std::vector<unsigned int> ebosToDelete;
};

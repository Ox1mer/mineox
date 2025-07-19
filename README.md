# 🛠️ MineOX — Infinite World Inspired by Minecraft and Terraria

**MineOX** is not just a chunk controller — it’s the foundation of my future game featuring an infinite world.  
Inspired by Minecraft and Terraria, MineOX generates chunks without limits up or down, creating a world that expands infinitely in all directions.

---

## 🌍 Project Features

- **Infinite World** — chunk generation with no height or depth limits  
- **Dynamic chunk loading** with multi-threaded streaming  
- **Custom block and texture system** with easy extensibility  
- **Optimized chunk memory management**  
- Inspired by Minecraft and Terraria, built with a custom architecture and engine

---

## 📦 Initial Setup and Launch

On the first run, MineOX will automatically create a configuration folder in your system:

- Configuration path:  %APPDATA%.mineox\data\textures\blocks
- (On Windows, `%APPDATA%` typically points to `C:\Users\<YourUserName>\AppData\Roaming`)

- If the `textures/blocks` folder does not exist, it will be created automatically.

- Block textures should be placed inside this folder as `.png` files.  
**Important:** Each texture file name must exactly match the block’s `stringRepresentation`.

---

## 🚀 How to Build and Run

1. Currently build in the VScode. Later CMAKE wil added. just clone the repository and build with your compiler (GCC for example)

---

c++20, OpenGL, GLFW, GLAD, stb_image

License
All rights reserved. See LICENSE for details.

Contact:
oximeracer@gmail.com
https://github.com/Ox1mer

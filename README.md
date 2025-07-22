# 🛠️ MineOX — Infinite World Inspired by Minecraft and Terraria

MineOX is not just a chunk controller — it’s the foundation of my future game featuring an infinite world.  
Inspired by Minecraft and Terraria, MineOX generates chunks without limits up or down, creating a world that expands infinitely in all directions.

---

## 🌍 Project Features

- Infinite World — chunk generation with no height or depth limits  
- Dynamic chunk loading with multi-threaded streaming  
- Custom block and texture system with easy extensibility  
- Optimized chunk memory management  
- Built with a custom architecture and engine, inspired by Minecraft and Terraria

---

## 📦 Initial Setup and Launch

On the first run, MineOX will automatically create a configuration folder in your system: %APPDATA%.mineox\data\textures\blocks

(On Windows, `%APPDATA%` typically points to `C:\Users\<YourUserName>\AppData\Roaming`)

If the `textures/blocks` folder does not exist, it will be created automatically.

Block textures should be placed inside this folder as `.png` files.  
**Important:** Each texture file name must exactly match the block’s `stringRepresentation`.

---

## 🚀 How to Build and Run

Currently, the project is built using VS Code tasks. CMake support will be added later.  

Steps to build:  
- Clone the repository  
- Build with your compiler (e.g. GCC) using the VS Code build tasks  

The project uses:  
C++20, OpenGL, GLFW, GLAD, stb_image

---

## 🔒 License

This project is licensed under a proprietary license.

**You may:**
- Download and view the source code
- Compile and run the project for personal use
- Modify the source code for personal, non-commercial experimentation

**You may not:**
- Publish or distribute the source code or your modifications
- Use the code or modifications in other projects or products
- Use the project for commercial purposes without permission

**Want to contribute?**
If you want to contribute your changes, please submit them for review. If approved, your contribution may be merged and you may be credited as a contributor.

See [LICENSE](./LICENSE) for details.

---

## 📬 Contact

Email: oximeracer@gmail.com  
GitHub: [https://github.com/Ox1mer](https://github.com/Ox1mer)

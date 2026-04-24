# 🎮 Pac-Man Game (Graphics Lab Project)

A classic **Pac-Man style arcade game** developed using **OpenGL & GLUT** for my Graphics Lab course.  
This project demonstrates fundamental computer graphics concepts like rendering, transformations, and user interaction.

---

## 🚀 Features

- 🟡 Smooth Pac-Man movement
- 👻 Intelligent Ghost AI (basic chasing logic)
- 💥 Collision Detection (Pac-Man vs Ghost)
- 🍒 Food / Pellet collection system
- 📊 Score Tracking
- 🎨 2D Graphics rendering using OpenGL
- ⌨️ Keyboard controls

---

## 🧠 Concepts Used

- OpenGL primitives (GL_POINTS, GL_POLYGON)
- 2D Transformations (Translation, Rotation)
- Event Handling (Keyboard Input)
- Animation using `glutIdleFunc()`
- Collision Detection Logic
- Game Loop Concept

---

## 🛠️ Technologies Used

- Language: **C/C++**
- Graphics Library: **OpenGL**
- Utility Toolkit: **GLUT**

---

## 🎮 Controls

| Key        | Action        |
|------------|--------------|
| Arrow Keys | Move Pac-Man |
| ESC        | Exit Game    |

---

## 📸 Preview

> (Add screenshots here after running your game)

---

## ⚙️ How to Run

### 🔹 Step 1: Install Dependencies
- Install OpenGL
- Install GLUT (or FreeGLUT)

### 🔹 Step 2: Compile

```bash
gcc pacman.c -lGL -lGLU -lglut -o pacman

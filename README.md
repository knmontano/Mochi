# 🐾 Mochi: The Chubby Orange Tabby Cat

A cozy, interactive desktop simulator featuring **Mochi**, a round and fluffy orange tabby cat. Built in pure C using the native Windows GDI API with zero external game engine dependencies.

---

## 🎮 Play & Download
* 🌐 **[Play Directly in Browser](https://knmontano.github.io/Mochi/)**
* 💻 **[Download Native cat.exe](https://raw.githubusercontent.com/knmontano/Mochi/main/cat.exe)**

---

## ✨ Features
* **Custom Vector GDI Art**: Pure procedural code rendering for Mochi, her sunny room, an aquarium goldfish, and a ceramic feeding bowl.
* **Interactive Feeding & Toys**: Give fresh salmon cuts, flick pink yarn balls across the floor, or drop down an Amazon cardboard box.
* **Direct Pet Reactions**:
  * Tap her pink nose to **BOOP**.
  * Squish her cheeks for purrs.
  * Rub her soft cream tummy (watch out for the 50/50 bunny-kick trap!).
* **Chonk Progression**: Mochi gains weight in kilograms as you feed her treats.

---

## 🛠️ Build Locally

Compile using GCC (MinGW / w64devkit):

```powershell
gcc main.c -o cat.exe -mwindows
.\cat.exe
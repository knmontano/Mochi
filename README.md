# 🐾 Mochi: The Chubby Cat

A cozy, interactive desktop simulator featuring **Mochi**, a round and fluffy orange tabby cat. Built in pure C using the native Windows GDI API with zero external game engine dependencies.

---

## 🎮 Play & Download
* 🌐 **[Play Directly in Browser](https://knmontano.github.io/Mochi/)**
* 💻 **[Download Native cat.exe](https://raw.githubusercontent.com/knmontano/Mochi/main/cat.exe)**

---

## ✨ Features
* **Custom Vector GDI Art**: Procedural rendering for Mochi, her sunny room (curtains, gradient sky), an aquarium goldfish, and a ceramic feeding bowl. Day and night themes.
* **Interactive Feeding & Toys**: Give fresh salmon cuts, flick pink yarn balls across the floor, or drop down an Amazon cardboard box.
* **Direct Pet Reactions**:
  * Tap her pink nose to **BOOP**.
  * Squish her cheeks for purrs.
  * Rub her soft cream tummy (watch out for the 50/50 bunny-kick trap!).
* **Chonk Progression**: Mochi gains weight as you feed her and earns a new title (Fluffy Loaf → Chonky → Extra Chonk → Absolute Unit → Mega Chonk). Playing burns a little off.

### New in the native build
* **Living needs**: Hunger, energy and happiness slowly drop over time. Meters turn red when low and Mochi meows when she's hungry.
* **Real naps**: She falls asleep when exhausted (or when you press Nap), recovers energy, and wakes on her own. Click her to wake her sooner.
* **Saves your progress**: Stats, weight, lighting and mute are stored in `%LOCALAPPDATA%\MochiCat.sav`. Time away is taken into account (gently, she never starves).
* **Smooth sound**: Sound effects no longer freeze the animation, and can be muted.
* **Polish**: Button hover and pressed states, a hand cursor over clickable things, blinking, rosy cheeks when she's happy, and a readable night mode.

---

## ⌨️ Controls

| Input | Action |
|-------|--------|
| `1` – `6` | Treat, Yarn, Box, Catnip, Zoomies, Nap |
| `L` or click the lamp | Toggle day / night |
| `M` or the speaker icon | Mute / unmute |
| Click Mochi | Boop, squish cheeks, rub tummy (or wake her up) |

---

## 🛠️ Build Locally

Compile using GCC (MinGW / w64devkit):

```powershell
gcc main.c -o cat.exe -mwindows -O2
.\cat.exe
```

In VS Code, **Ctrl+Shift+B** runs the included build task, and **F5** builds and launches under gdb.
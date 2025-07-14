# TP Final - Space Invaders 👾

This is a Space Invaders game recreation. The game can be run on a PC, but also on a Raspberry PI with a 16x16 LED matrix connected as the "screen", and a joystick with a button for player control.

---

## 🎮 Controls

### ⌨ For PC

Arrow left/right: Move

X: Shoot

ESC: Pause / Resume

### 🕹️ For RPI

Stick left/right: Move

Button: Shoot
Also hold the button to go into pause

---

## 📦 Dependencies

You need to have Allegro 5 installed in order to compile the game. This applies to the PC version only. Raspberry PI version does not need this.

### On Debian-based systems (Ubuntu, Linux Mint, etc.):

```bash
sudo apt install liballegro5-dev
```

---

## ⚙️ How to Compile

Open a terminal and run the following commands:

```bash
cd src
make
```

The makefile will automatically decide whether to compile for a PC or Raspberry PI based on the environment.

---

## 🚀 How to Run

After compiling, run the game with:

```bash
./SpaceInvaders
```

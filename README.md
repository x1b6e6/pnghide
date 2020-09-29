# pnghide
hide plain text in png file inside pixels (unvisible changes).

## How to use

Example: I want hide `data.txt` file inside `image.png` and store result in `result.png`:
```bash
pnghide hide image.png --input=data.txt --output=result.png
```

Example: I want hide some text inside `image.png` and store result in `result.png`:
```bash
pnghide hide image.png --output=result.png
```
Enter your text to program.
Press `Ctrl+D` for finish.

Example: I want get back my `data.txt` from `result.png`:
```bash
pnghide unhide result.png --output=data.txt
```

Example: I want get back my some text from `result.png` (print to terminal):
```bash
pnghide unhide result.png
```

## How to build and install
CMake:
```bash
cmake -Bbuild
cmake --build build
./build/pnghide # is result file
```


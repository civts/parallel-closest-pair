# Parallel Closest Pair of Points

🏛 University project for the "High Performance Computing" course by Sandro Luigi Fiore

## Using the program 💽

### Input format

A text file where the first row contains the number of points (unsigned int32).
Each subsequent row contains two integers x and y (int32) in the format `x<space>y`,
representing the coordinates of a point.

Example of a file with points `[(1, 2), (-3, 89), (42, 137)]`:

```text
3
1 2
-3 89
42 137
```

### Output format

```text
Found the closest pair in 100 milliseconds.
The closest pair of points is:
(-3, 89) and (42, 137)
Their distance is:
65.79513
```

## Dev environment setup 👨‍💻

### Option 1: Manual 🔨

1. Install the following packets:

   - gcc (we used version [12.2.0](https://repology.org/project/gcc/versions))
   - glibc (we used version [2.35-163](https://repology.org/project/glibc/versions))
   - clang-format (we used version [11.1.0](https://clang.llvm.org/docs/ClangFormat.html))

1. If you are using [VSCode](https://code.visualstudio.com)/[VSCodium](https://vscodium.com) as editor,
   we recommend you have the following extensions installed:

   - [vadimcn.vscode-lldb](https://open-vsx.org/extension/vadimcn/vscode-lldb/1.7.4) (we used version 1.7.4)
   - [xaver.clang-format](https://open-vsx.org/extension/xaver/clang-format/1.9.0) (we used version 1.9.0)
   - [ms-vscode.cpptools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) (we used version 1.11.0)
   - [franneck94.c-cpp-runner](https://open-vsx.org/extension/franneck94/c-cpp-runner/3.0.0) (we used version 3.0.0)

1. Clone this repo, open it in the editor

1. Open the file you want to debug and press `F5` to launch it.

### Option 2: With Nix ❄️

With [Nix](https://nixos.org) and [direnv](https://direnv.net) you can get exactly our setup by running this script:

```bash
git clone https://github.com/civts/parallel-closest-pair #Downloads this repository
cd parallel-closest-pair
direnv allow #Fetches all the dependencies (and VSCodium)
codium . #Launches VSCodium
```

Then open the file you want to debug and press `F5` to launch it.

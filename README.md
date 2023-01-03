# Parallel Closest Pair of Points

🏛 University project for the "High Performance Computing" course by Sandro Luigi Fiore

## Using the program 💽

### Connecting to the cluster

You'll need a VPN to connect to Trento's VPN to access `hpc2.unitn.it`

1.  Open the GlobalProtect VPN client by running `gpclient` (be sure to follow the setup instructions from the [official repo](https://github.com/yuezk/GlobalProtect-openconnect))

1.  Enter as portal address `vpn.icts.unitn.it`

1.  The username is your unitn email without the "studenti" subdomain -e.g., `mario.rossi@unitn.it`-

1.  The password is your unitn password

1.  Now you should be able to successfully ping `hpc2.unitn.it`

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

### Running the program

#### Method 1: Github CI

In order to run this program on the cluster, just push here on Github on the main branch. The CI will:

- build the code

- copy it in the cluster

- start the job

- notify us on telegram once it is finished

Note that for this job to run successfully, the following secrets have to be present in the github repo:

- `BOT_TOKEN`: The token of the telegram bot used for notifying that the job completed
- `TELEGRAM_CHAT_ID`: The ID of the chat where the bot sends the updates
- `UNITN_PASSWORD`: Unitn email
- `UNITN_USERNAME`: Unitn password

#### Method 2: Manual

##### Getting the code to the cluster

1. Ensure you can ping the cluster (`hpc2.unitn.it`). Otherwise you may need to connect to the VPN.

1. Open a ssh connection to the cluster. You can do so by running `ssh hpc2.unitn.it -l mario.rossi@unitn.it`. Then note the location of your home directory on the cluster, given by the output of the `pwd` command (should be `/home/mario.rossi`).

1. Copy the code to the cluster by either
   1. Running on your machine
      `scp -r -o "user=mario.rossi@unitn.it" ./path/to/the/code/on/your/machine hpc2.unitn.it:/home/mario.rossi/HPC_course`
   1. Running on the cluster -in the target directory- `git clone https://github.com/civts/parallel-closest-pair`
1. Compile the code by running (on the cluster) `module load mpich-3.2`, then `mpicc -o parallel_closest_points main.c -lm`

##### Running the program

Start a run of the program (job) by running on the cluster `qsub

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

## Troubleshooting

### VS Code

If it does not find `gtest/gtest.h`, run `./scripts/test.sh`, ensure that the `test/build` is shown in the file explorer on the side (otherwise modify `files.exclude` in `settings.json` to not exclude it). Then restart VS Code.

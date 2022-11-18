{
  description = "Dev environment with all you need to develop in C with VSCode/Codium";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config = {
            allowUnfree = true;
          };
        };
      in
      {
        devShell = with pkgs; mkShell rec {
          nativeBuildInputs = [
            gcc12
            glibc
            mpi
            clang #Required by xaver.clang-format
            (vscode-with-extensions.override {
              vscode = vscodium;
              vscodeExtensions = with vscode-extensions; [
                #C extensions
                vadimcn.vscode-lldb
                ms-vscode.cpptools
                #Formatters
                xaver.clang-format
                esbenp.prettier-vscode
                #Nix specific
                jnoortheen.nix-ide
              ] ++ pkgs.vscode-utils.extensionsFromVscodeMarketplace [
                {
                  name = "c-cpp-runner";
                  publisher = "franneck94";
                  version = "3.0.0";
                  sha256 = "sha256-huI1qtBfBjD8N7t3HFnoX8kGcCNv8pDLOr0l3/0gD84=";
                }
              ];
            })
          ];
          shellHook = ''
            #Add the MPI header files to the workspace so that they can be found by gcc/mpicc
            ln -sfn ${mpi}/include ./.mpi_include
          '';
        };
      });
}

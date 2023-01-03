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
        openVsxUtils = import ./openvsx.nix { inherit pkgs; };
      in
      {
        devShell = with pkgs; mkShell rec {
          nativeBuildInputs = [
            gcc12
            glibc
            mpi
            clang #Required by xaver.clang-format
            shfmt #Required by foxundermoon.shell-format
            (vscode-with-extensions.override {
              vscode = vscodium;
              vscodeExtensions = with vscode-extensions; [
                #C extensions
                vadimcn.vscode-lldb
                ms-vscode.cpptools
                #Formatters
                xaver.clang-format
                esbenp.prettier-vscode
                foxundermoon.shell-format
                #Nix specific
                jnoortheen.nix-ide
              ] ++ pkgs.vscode-utils.extensionsFromVscodeMarketplace [
                {
                  name = "c-cpp-runner";
                  vscodeExtName = "c-cpp-runner";
                  publisher = "franneck94";
                  vscodeExtPublisher = "franneck94";
                  version = "3.0.0";
                  sha256 = "sha256-YKEW6nvLLvkxjqZenztcf5zpbMPTqV1NYGVo19Fqv8I=";
                }
              ]
              ++ openVsxUtils.extensionsFromOpenvsxMarketplace [
                {
                  name = "open-remote-ssh";
                  vscodeExtName = "open-remote-ssh";
                  publisher = "jeanp413";
                  vscodeExtPublisher = "jeanp413";
                  version = "0.0.21";
                  sha256 = "sha256-/khL21yn/85AIb54bpCnHxfbjCwsr96B+FNmIG8qBY4=";
                }
              ]
              ;
            })
          ];
          shellHook = ''
            #Add the MPI header files to the workspace so that they can be found by gcc/mpicc
            ln -sfn ${mpi}/include ./.mpi_include
          '';
        };
      });
}

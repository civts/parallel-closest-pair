{ pkgs }:
let
  buildVscodeExtension =
    a@{ name
    , src
    , # Same as "Unique Identifier" on the extension's web page.
      # For the moment, only serve as unique extension dir.
      vscodeExtUniqueId
    , configurePhase ? ''
        runHook preConfigure
        runHook postConfigure
      ''
    , buildPhase ? ''
        runHook preBuild
        runHook postBuild
      ''
    , dontPatchELF ? true
    , dontStrip ? true
    , nativeBuildInputs ? [ ]
    , ...
    }:
    pkgs.stdenv.mkDerivation ((removeAttrs a [ "vscodeExtUniqueId" ]) // {

      name = "vscode-extension-${name}";

      inherit vscodeExtUniqueId;
      inherit configurePhase buildPhase dontPatchELF dontStrip;

      installPrefix = "share/vscode/extensions/${vscodeExtUniqueId}";

      nativeBuildInputs = [ pkgs.unzip ] ++ nativeBuildInputs;

      installPhase = ''
        runHook preInstall

        mkdir -p "$out/$installPrefix"
        find . -mindepth 1 -maxdepth 1 | xargs -d'\n' mv -t "$out/$installPrefix/"

        runHook postInstall
      '';
    });

  buildOpenvsxMarketplaceExtension =
    a@{ name ? ""
    , src ? null
    , vsix ? null
    , mktplcRef
    , ...
    }: assert "" == name; assert null == src;
    buildVscodeExtension ((removeAttrs a [ "mktplcRef" "vsix" ]) // {
      name = "${mktplcRef.publisher}-${mktplcRef.name}-${mktplcRef.version}";
      version = mktplcRef.version;
      src =
        if (vsix != null)
        then vsix
        else fetchVsixFromOpenvsxMarketplace mktplcRef;
      vscodeExtUniqueId = "${mktplcRef.publisher}.${mktplcRef.name}";
    });

  mktplcRefAttrList = [
    "name"
    "publisher"
    "version"
    "sha256"
    "arch"
  ];

  mktplcExtRefToExtDrv = ext:
    buildOpenvsxMarketplaceExtension (removeAttrs ext mktplcRefAttrList // {
      mktplcRef = builtins.intersectAttrs (pkgs.lib.genAttrs mktplcRefAttrList (_: null)) ext;
    });

  extensionFromOpenvsxMarketplace = mktplcExtRefToExtDrv;
  extensionsFromOpenvsxMarketplace = mktplcExtRefList:
    builtins.map extensionFromOpenvsxMarketplace mktplcExtRefList;

  fetchVsixFromOpenvsxMarketplace =
    let
      toFetchUrlArgs = { publisher, name, version, arch ? "", sha256 }:
        let
          archurl = (if arch == "" then "" else "/${arch}");
        in
        {
          url = "https://open-vsx.org/api/${publisher}/${name}${archurl}/${version}/file/${publisher}.${name}-${version}.vsix";
          name = "${publisher}-${name}.zip";
          inherit sha256;
        };
    in
    (mktplcExtRef: pkgs.fetchurl (toFetchUrlArgs mktplcExtRef));
in
{
  inherit fetchVsixFromOpenvsxMarketplace buildVscodeExtension
    buildOpenvsxMarketplaceExtension extensionFromOpenvsxMarketplace
    extensionsFromOpenvsxMarketplace;
}

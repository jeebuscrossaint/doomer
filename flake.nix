{
  description = "C project with Raylib, Grim, and GCC";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = {
    self,
    nixpkgs,
  }: {
    devShells.x86_64-linux.default = nixpkgs.legacyPackages.x86_64-linux.mkShell {
      buildInputs = [
        nixpkgs.legacyPackages.x86_64-linux.gcc
        nixpkgs.legacyPackages.x86_64-linux.raylib
        nixpkgs.legacyPackages.x86_64-linux.grim
        nixpkgs.legacyPackages.x86_64-linux.pkg-config
        nixpkgs.legacyPackages.x86_64-linux.wlroots
      ];

      shellHook = ''
        echo "C development environment with Raylib, Grim, and GCC is ready!"
      '';
    };

    packages.x86_64-linux.default = nixpkgs.legacyPackages.x86_64-linux.stdenv.mkDerivation {
      pname = "doomer";
      version = "1.0";
      src = ./.;

      nativeBuildInputs = [nixpkgs.legacyPackages.x86_64-linux.pkg-config];
      buildInputs = [nixpkgs.legacyPackages.x86_64-linux.raylib];

      buildPhase = ''
        gcc -o doomer doomer.c -lraylib -lm -lpthread -ldl -O3 -march=native -flto
        strip doomer
      '';

      installPhase = ''
        mkdir -p $out/bin
        mv doomer $out/bin/
      '';
    };
  };
}

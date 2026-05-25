{
    description = "";

    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
        flake-parts.url = "github:hercules-ci/flake-parts";
    };

    outputs =
        inputs@{ self, flake-parts, ... }:
        flake-parts.lib.mkFlake { inherit inputs; } {
            systems = [
                "x86_64-linux"
                "aarch64-linux"
                "aarch64-darwin"
                "x86_64-darwin"
            ];

            imports = [ ];

            perSystem =
                {
                    config,
                    self',
                    inputs',
                    pkgs,
                    final,
                    system,
                    ...
                }:
                {
                    devShells.default = pkgs.mkShell {
                        name = "cdm";

                        packages = with pkgs; [
                            gcc15
                            gdb
                            pkg-config
                            gnumake

                            # BLE
                            sdbus-cpp
                            bluez

                            # UI
                            qt6.qtbase
                            qt6.qtsvg

                            # For working with the apk
                            apktool
                            jadx
                            jdk
                        ];

                        shellHook = ''
                            export PATH="${pkgs.qt6.qtbase}/libexec:$PATH"
                        '';
                    };
                };

            flake = {
                homeModules.default =
                    { config, pkgs, ... }:
                    {
                        imports = [ ];
                    };
            };
        };
}

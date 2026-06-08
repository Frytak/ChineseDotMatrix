{
    description = "";

    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
        flake-parts.url = "github:hercules-ci/flake-parts";
        nix-appimage.url = "github:ralismark/nix-appimage";
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
                    packages.cdm = pkgs.stdenv.mkDerivation {
                        pname = "cdm";
                        version = "0.1.0";
                        src = ./.;

                        meta = {
                            mainProgram = "cdm";
                        };

                        nativeBuildInputs = with pkgs; [
                            pkg-config
                            gnumake
                            gcc15
                            qt6.wrapQtAppsHook # CRITICAL: Wraps the binary with Qt environment variables
                        ];

                        buildInputs = with pkgs; [
                            qt6.qtbase
                            qt6.qtsvg
                            sdbus-cpp_2
                            bluez
                        ];

                        # Tell make to install to the Nix output directory
                        makeFlags = [
                            "PREFIX=${placeholder "out"}"
                            "BUILD_TYPE=release"
                            "MOC=${pkgs.qt6.qtbase}/libexec/moc"
                            "RCC=${pkgs.qt6.qtbase}/libexec/rcc"
                        ];
                    };

                    packages.default = self'.packages.cdm;

                    # 3. Create the AppImage bundler target
                    packages.appimage = inputs.nix-appimage.bundlers.${system}.default self'.packages.default;

                    devShells.default = pkgs.mkShell {
                        name = "cdm";

                        packages = with pkgs; [
                            gcc15
                            gdb
                            pkg-config
                            gnumake

                            # BLE
                            sdbus-cpp_2
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

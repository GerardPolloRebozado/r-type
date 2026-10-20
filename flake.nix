{
  description = "RTypeEditor — visual editor for the R-Type engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs = { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system);

      mkRTypeEditor = pkgs:
        let
          inherit (pkgs) lib;
          isLinux = pkgs.stdenv.hostPlatform.isLinux;

          # Pinned sources normally downloaded by CPM at configure time.
          raylibSrc = pkgs.fetchFromGitHub {
            owner = "raysan5";
            repo = "raylib";
            rev = "8658e4e55123c3354fe2a8c8583956ec6983bcc5";
            hash = "sha256-p2f0vMF9s8mgcAuilT6tx+//9HjLJpO12khkrtRbWgY=";
          };

          raylibCppSrc = pkgs.fetchFromGitHub {
            owner = "RobLoach";
            repo = "raylib-cpp";
            rev = "798b4c3591c294b2224a02e17be7eb57ed7dceed";
            hash = "sha256-I0sF9PFb6Zsu34KrYPj3FEC7oX5icoy6upuz915oMsA=";
          };

          imguiSrc = pkgs.fetchFromGitHub {
            owner = "ocornut";
            repo = "imgui";
            rev = "d88f238d1d363ef387382b2e29ad12cf96f826ac";
            hash = "sha256-qmShzZ1YiRqsdWkKREax65jY5CTUlBFzZnC4tPJ98+0=";
          };

          rlImGuiSrc = pkgs.fetchFromGitHub {
            owner = "raylib-extras";
            repo = "rlImGui";
            rev = "1550009359ad975927f7f0e4a3f47e3f27123ea9";
            hash = "sha256-1pN236S3R8S2PrwhFgNfxJCuUfdGG/Xg7lqdl83mXFM=";
          };

          enetSrc = pkgs.fetchFromGitHub {
            owner = "zpl-c";
            repo = "enet";
            rev = "8b43b92591fa9458662262a598b092ecc5132865";
            hash = "sha256-/CY6j5v3A8dW2PE/Rv+Z6WsDxENFGoeOUZ4mONZHJD0=";
          };

          glazeSrc = pkgs.fetchFromGitHub {
            owner = "stephenberry";
            repo = "glaze";
            rev = "d78832c82289c61a9315bfbc35332cec9f4e93ca";
            hash = "sha256-dzvKhaSfaDuJ+yHep+OgC6kTRMW59kOtSDw2KD1drVI=";
          };

          sol2Src = pkgs.fetchFromGitHub {
            owner = "ThePhD";
            repo = "sol2";
            rev = "dca62a0f02bb45f3de296de3ce00b1275eb34c25";
            hash = "sha256-7QHZRudxq3hdsfEAYKKJydc4rv6lyN6UIt/2Zmaejx8=";
          };

          cpmSrc = pkgs.fetchurl {
            url = "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.0/CPM.cmake";
            hash = "sha256-ezVPOll2xGJsh2hQyTlE5SyD7FmhWa5d5b55g/Dheio=";
          };

          pfdSrc = pkgs.fetchurl {
            url = "https://raw.githubusercontent.com/samhocevar/portable-file-dialogs/c12ea8c9a727f5320a2b4570aee863bbede2a204/portable-file-dialogs.h";
            hash = "sha256-XLHm5jr1iKJ0xnZRm+TNdjzneQRjLFilsTk7Q40cfGo=";
          };

          runtimeLibs = with pkgs; [
            glfw
            lua5_4
          ] ++ lib.optionals isLinux (with pkgs; [
            libGL
            libGLU
            libx11
            libxrandr
            libxcursor
            libxi
            libxinerama
            libxext
            libxxf86vm
            wayland
            libxkbcommon
            libdecor
            alsa-lib
            libpulseaudio
          ]);
        in
        pkgs.stdenv.mkDerivation {
          pname = "rtype-editor";
          version = self.shortRev or self.dirtyShortRev or "dev";

          src = self;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
            makeWrapper
            git
          ];

          buildInputs = with pkgs; [
            glfw
            lua5_4
          ] ++ lib.optionals isLinux (with pkgs; [
            libGL
            libGLU
            libx11
            libxrandr
            libxcursor
            libxi
            libxinerama
            libxext
            libxxf86vm
            wayland
            libxkbcommon
            libdecor
            alsa-lib
            libpulseaudio
          ]);

          # Make CMake use the pinned sources instead of downloading them.
          cmakeFlags = [
            "-GNinja"
            "-DUSE_EXTERNAL_GLFW=ON"
            "-DCPM_raylib_SOURCE=${raylibSrc}"
            "-DCPM_raylib-cpp_SOURCE=${raylibCppSrc}"
            "-DCPM_imgui_SOURCE=${imguiSrc}"
            "-DCPM_rlImGui_SOURCE=${rlImGuiSrc}"
            "-DCPM_enet_SOURCE=${enetSrc}"
            "-DCPM_glaze_SOURCE=${glazeSrc}"
          ];

          # Replace the two runtime downloads with local files.
          postPatch = ''
            substituteInPlace CMakeLists.txt \
              --replace-fail \
                'https://github.com/cpm-cmake/CPM.cmake/releases/download/v''${CPM_DOWNLOAD_VERSION}/get_cpm.cmake' \
                'file://${cpmSrc}'

            substituteInPlace Editor/CMakeLists.txt \
              --replace-fail \
                'https://raw.githubusercontent.com/samhocevar/portable-file-dialogs/master/portable-file-dialogs.h' \
                'file://${pfdSrc}'
          '';

          # The project patches sol2 at configure time, so give it a writable copy.
          preConfigure = ''
            cp -r ${sol2Src} "$NIX_BUILD_TOP/sol2-src"
            chmod -R u+w "$NIX_BUILD_TOP/sol2-src"
            cmakeFlagsArray+=("-DCPM_sol2_SOURCE=$NIX_BUILD_TOP/sol2-src")
          '';

          buildPhase = ''
            runHook preBuild
            cmake --build . --target RTypeEditor --parallel
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            install -Dm755 RTypeEditor "$out/bin/RTypeEditor"
            runHook postInstall
          '';

          postFixup = lib.optionalString isLinux ''
            wrapProgram "$out/bin/RTypeEditor" \
              --prefix LD_LIBRARY_PATH : ${lib.makeLibraryPath runtimeLibs}
          '';

          meta = {
            description = "Visual editor for the R-Type engine";
            homepage = "https://github.com/GerardPolloRebozado/r-type";
            mainProgram = "RTypeEditor";
            platforms = lib.platforms.linux ++ lib.platforms.darwin;
          };
        };
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        rec {
          default = rtype-editor;
          rtype-editor = mkRTypeEditor pkgs;
        });

      apps = forAllSystems (system: {
        default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/RTypeEditor";
        };
      });

      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          inherit (pkgs) lib;
        in
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              cmake
              ninja
              pkg-config
              git
              glfw
              lua5_4
            ] ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux (with pkgs; [
              libGL
              libGLU
              libx11
              libxrandr
              libxcursor
              libxi
              libxinerama
              libxext
              libxxf86vm
              wayland
              libxkbcommon
              libdecor
              alsa-lib
              libpulseaudio
            ]);

            LD_LIBRARY_PATH = lib.makeLibraryPath (
              with pkgs; [
                glfw
                lua5_4
              ] ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux (with pkgs; [
                libGL
                libGLU
                libx11
                libxrandr
                libxcursor
                libxi
                libxinerama
                libxext
                libxxf86vm
                wayland
                libxkbcommon
                libdecor
                alsa-lib
                libpulseaudio
              ])
            );
          };
        });
    };
}

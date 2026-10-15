{
  pkgs,
  lib,
  config,
  inputs,
  ...
}:

{
  # https://devenv.sh/basics/
  env.GREET = "devenv";
  env.LD_LIBRARY_PATH = lib.makeLibraryPath [
    pkgs.libpulseaudio
    pkgs.alsa-lib
    pkgs.libGL
    pkgs.xorg.libX11
    pkgs.xorg.libXrandr
    pkgs.xorg.libXi
    pkgs.xorg.libXcursor
    pkgs.wayland
    pkgs.lua5_4
  ];

  # https://devenv.sh/packages/
  packages = [
    pkgs.cmake
    pkgs.autoconf
    pkgs.autoconf-archive
    pkgs.automake

    # OpenGL & Display Libraries
    pkgs.libGL
    pkgs.libGLU
    pkgs.xorg.libX11
    pkgs.xorg.libXcursor
    pkgs.xorg.libXinerama
    pkgs.xorg.libXrandr
    pkgs.xorg.libXi
    pkgs.xorg.libXxf86vm
    pkgs.wayland
    pkgs.wayland-protocols
    pkgs.libxkbcommon
    
    # Audio Libraries
    pkgs.libpulseaudio
    pkgs.alsa-lib
    pkgs.xxd
    pkgs.doxygen

    pkgs.lefthook
    pkgs.lua5_4
  ];

  # https://devenv.sh/languages/
  languages = {
    c.enable = true;
  };

  languages.python.enable = true;
  languages.python.venv.enable = true;

  # https://devenv.sh/processes/
  # processes.dev.exec = "${lib.getExe pkgs.watchexec} -n -- ls -la";

  # https://devenv.sh/services/
  # services.postgres.enable = true;

  # https://devenv.sh/scripts/
  scripts.hello.exec = ''
    echo hello from $GREET
  '';

  # https://devenv.sh/basics/
  enterShell = ''
    hello         # Run scripts directly
  '';

  # https://devenv.sh/tasks/
  # tasks = {
  #   "myproj:setup".exec = "mytool build";
  #   "devenv:enterShell".after = [ "myproj:setup" ];
  # };

  # https://devenv.sh/tests/
  enterTest = ''
    echo "Running tests"
    git --version | grep --color=auto "${pkgs.git.version}"
  '';

  # https://devenv.sh/git-hooks/
  # git-hooks.hooks.shellcheck.enable = true;

  # See full reference at https://devenv.sh/reference/options/
}

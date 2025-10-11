let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-25.05";
  pkgs = import nixpkgs { config = {}; overlays = []; };
in

pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }
{  
  packages = with pkgs; [
    libGL
    sdl3
    xorg.libX11
  ];  
}


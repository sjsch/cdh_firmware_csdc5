with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "cdh_firmware_csdc5";
  version = "dev";
  src = ./.;

  buildInputs = [
    gcc-arm-embedded
    ninja
    cmake
    openocd
  ];

  configurePhase = ''
    ./boot.sh
  '';

  installPhase = ''
    mkdir -p $out
    mv bin/*.elf $out
  '';
}

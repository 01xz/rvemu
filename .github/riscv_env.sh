export PATH=$RISCV/bin:$PATH

TARGET=riscv64-unknown-elf

mkdir -p $RISCV

curl -Lo /tmp/riscv.tar.gz https://github.com/riscv-collab/riscv-gnu-toolchain/releases/download/2023.07.07/riscv64-elf-ubuntu-22.04-gcc-nightly-2023.07.07-nightly.tar.gz

tar -xvf /tmp/riscv.tar.gz -C $RISCV --strip-components 1

git clone https://github.com/riscv-software-src/riscv-tests.git /tmp/riscv-tests

cd /tmp/riscv-tests
git submodule update --init --recursive
autoconf
autoupdate
./configure --prefix=$RISCV/$TARGET
make -j `nproc`
make install

FROM scratch
COPY --from=stagex/core-filesystem . /
COPY --from=stagex/core-busybox . /
COPY --from=stagex/core-binutils . /
COPY --from=stagex/core-bash . /
COPY --from=stagex/core-make . /
COPY --from=stagex/core-cmake . /
COPY --from=stagex/core-python . /
COPY --from=stagex/core-py-setuptools . /
COPY --from=stagex/core-zlib . /
COPY --from=stagex/core-pkgconf . /
COPY --from=stagex/core-openssl . /
COPY --from=stagex/core-perl . /
COPY --from=stagex/core-gcc . /
COPY --from=stagex/core-curl . /
COPY --from=stagex/core-libunwind . /
COPY --from=stagex/core-musl . /
COPY --from=stagex/core-llvm16 . /
ADD https://github.com/rust-lang/rust/archive/refs/tags/1.74.0.tar.gz /usr/src/rustc-1.74.0-src.tar.gz
COPY --chmod=755 <<-EOF /usr/bin/build-mrustc
	set -eux
	export ARCH="$(uname -m)"
	export MRUSTC_TARGET_VER=1.74
	export RUSTC_VERSION=1.74.0
	export MRUSTC_DEBUG=Expand
	export MRUSTC_DUMP_PROCMACRO=dump_prefix
	export RUSTC_INSTALL_BINDIR=bin
	export OUTDIR_SUF=
	export RUSTC_TARGET="${ARCH}-unknown-linux-musl"
	cp /usr/src/rustc-1.74.0-src.tar.gz .
	make -j "$(nproc)"
	make -j "$(nproc)" -f minicargo.mk LIBS
	make -j "$(nproc)" test local_tests
	make -j "$(nproc)" -f minicargo.mk LLVM_CONFIG=/usr/bin/llvm-config output/rustc
	make -j "$(nproc)" -f minicargo.mk LLVM_CONFIG=/usr/bin/llvm-config output/cargo
	make -j "$(nproc)" -C run_rustc LLVM_CONFIG=/usr/bin/llvm-config
	mkdir ../rust-1.74.0
	cp -R run_rustc/output/prefix ../rust-1.74.0/usr
EOF
WORKDIR /home/user
ENTRYPOINT ["/usr/bin/bash"]

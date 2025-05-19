FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential wget git cmake \
    libssl-dev liblua5.3-dev libyara-dev \
    libglib2.0-dev libnice-dev pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install premake5
RUN wget -q https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-linux.tar.gz \
    && tar -xzf premake-5.0.0-beta2-linux.tar.gz \
    && mv premake5 /usr/local/bin \
    && rm premake-5.0.0-beta2-linux.tar.gz

# Install liboqs
RUN git clone --branch main --depth 1 https://github.com/open-quantum-safe/liboqs.git /tmp/liboqs \
    && mkdir /tmp/liboqs/build && cd /tmp/liboqs/build \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr -DOQS_BUILD_SHARED_LIBS=ON .. \
    && make -j$(nproc) && make install \
    && rm -rf /tmp/liboqs

WORKDIR /app
COPY . .

# Copy Lua script into the container
COPY script.lua /app/

# Build with error checking
RUN set -eux; \
    premake5 gmake2; \
    make -C build config=release -j$(nproc); \
    mkdir -p /app/bin; \
    find /app/bin -type f -executable -exec chmod +x {} \; || true

# Smart entry that finds the executable
CMD ["/bin/bash", "-c", \
    "echo 'Contents of /app/bin:'; ls -lR /app/bin; \
    echo '\nLooking for executable...'; \
    executable=$(find /app/bin -type f -executable -name '*SecureChatApp*' -o -name '*.out' -o -name '*.bin' | head -1); \
    if [ -f \"$executable\" ]; then \
        echo \"Found executable: $executable\"; \
        $executable; \
    else \
        echo 'Error: No executable found in /app/bin'; \
        echo 'Build artifacts:'; \
        find /app/build -type f; \
        exit 1; \
    fi"]

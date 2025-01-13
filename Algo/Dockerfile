FROM ubuntu:22.04 as setup

# Install basic build tools and Clang
RUN apt-get update -y && \
    apt-get install -y clang make build-essential && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy source code and Makefiles
COPY ./Algo /app/Algo
COPY ./client /app/client
COPY ./mathlib /app/mathlib
COPY ./makefile /app/makefile

# Build the C++ application
RUN make

# Run the Bot
CMD ["./AlgoBot"]

# Dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    fuse \
    libfuse-dev \
    pkg-config \
    gcc \
    make 

WORKDIR /app
COPY antink.c /app/
RUN gcc -Wall antink.c `pkg-config fuse --cflags --libs` -o antink && \
    chmod +x /app/antink && \
    echo "user_allow_other" >> /etc/fuse.conf

RUN mkdir -p /antink_mount /it24_host /antink_logs

CMD ["/app/antink", "-f", "/antink_mount"]  

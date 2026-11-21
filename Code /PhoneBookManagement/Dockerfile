# Docker deployment for a Qt Widgets C++ desktop app (Linux container).
# The graphical application is displayed in your browser through noVNC.
FROM ubuntu:24.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools \
    libqt6sql6-sqlite && rm -rf /var/lib/apt/lists/*
WORKDIR /source
COPY . .
RUN cmake -S . -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build --parallel 2

FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    libqt6widgets6 libqt6sql6 libqt6network6 libqt6sql6-sqlite \
    qt6-qpa-plugins libxcb-cursor0 libgl1 \
    xvfb x11vnc openbox novnc websockify \
    fonts-dejavu-core ca-certificates \
    && rm -rf /var/lib/apt/lists/*
RUN useradd --create-home --shell /bin/bash phonebook \
    && mkdir -p /opt/phonebook /data \
    && chown phonebook:phonebook /opt/phonebook /data
COPY --from=builder --chown=phonebook:phonebook /build/PhoneBookManagement /opt/phonebook/PhoneBookManagement
COPY --chown=phonebook:phonebook docker/start.sh /opt/phonebook/start.sh
RUN chmod 755 /opt/phonebook/start.sh /opt/phonebook/PhoneBookManagement
USER phonebook
WORKDIR /opt/phonebook
ENV XDG_DATA_HOME=/data \
    QT_QPA_PLATFORM=xcb \
    QT_X11_NO_MITSHM=1
EXPOSE 6080
CMD ["/opt/phonebook/start.sh"]

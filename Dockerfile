FROM centos:centos7

# Development Tools
RUN yum update -y; \
    yum -y install centos-release-scl; \
    yum -y install epel-release; \
    yum install -y devtoolset-8; \
    yum install -y cmake3; \
    yum install -y git

# Solusek Dependencies

# OpenSSL
RUN yum install -y openssl-devel

# MariaDB
RUN curl -L https://downloads.mariadb.com/MariaDB/mariadb_repo_setup > mariadb_repo_setup; \
    chmod +x mariadb_repo_setup; \
    ./mariadb_repo_setup; \
    yum install -y MariaDB-devel

# Solusek Build
WORKDIR /opt/libs

COPY . /opt/solusek
WORKDIR /opt/solusek

RUN mkdir build; \
    cd build; \
    export CXX="/opt/rh/devtoolset-8/root/usr/bin/g++"; \
    cmake3 ..; \
    make; \
    make install

ENTRYPOINT [ "/bin/bash" ]
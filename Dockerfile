FROM everware/base

MAINTAINER Tim Head <betatim@gmail.com>

USER root
RUN apt-get -y --force-yes install libx11-dev libxpm-dev libxft-dev libxext-dev libpng3 libjpeg8 gfortran libssl-dev libpcre3-dev libgl1-mesa-dev libglew1.5-dev libftgl-dev libmysqlclient-dev libfftw3-dev libcfitsio3-dev graphviz-dev libavahi-compat-libdnssd-dev libldap2-dev libxml2-dev

WORKDIR /tmp

RUN /bin/bash -c "git clone --depth 1 http://root.cern.ch/git/root.git -b v5-34-21 --single-branch \
   && source activate py27 \
   && cd root \
   && ./configure --prefix=/usr/local --minimal --disable-x11 \
           --enable-astiff --enable-builtin-afterimage --enable-builtin_ftgl --enable-builtin_glew --enable-builtin_pcre --enable-builtin-lzma \
           --enable-python --enable-roofit --enable-xml --enable-minuit2 \
           --disable-xrootd --fail-on-missing \
   && make -j2 \
   && make install \
   && cd .. \
   && rm -rf root"

ENV LD_LIBRARY_PATH /usr/local/lib/root:$LD_LIBRARY_PATH
ENV PYTHONPATH /usr/local/lib/root:$PYTHONPATH

WORKDIR /home/jupyter
USER jupyter

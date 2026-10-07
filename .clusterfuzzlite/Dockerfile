# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:8a03650a9d44a382fb126c2239790ef078c82bcd39439625233e8ca74203eb53

COPY . $SRC/FunctionPro
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FunctionPro

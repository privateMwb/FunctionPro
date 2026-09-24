# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:d0d9fb8232fd746e487e6e8e6f8631060b4a391a64b19ad3d87bdf454d00da50

COPY . $SRC/FunctionPro
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FunctionPro

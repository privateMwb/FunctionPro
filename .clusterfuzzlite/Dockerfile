# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:e67da2b0f6156b29b62734305a8c74db61aaf3158c7ab96386a07974fb33cdc8

COPY . $SRC/FunctionPro
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FunctionPro

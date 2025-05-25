#!/bin/bash
# run.sh - esp 디렉토리 -> FAT32 이미지 -> QEMU 자동 실행

set -e

ESP_DIR="esp"
IMG_FILE="esp.img"
IMG_SIZE_MB=512
MOUNT_POINT="/mnt/tmp_esp"

echo "[1/4] 기존 esp.img 삭제"
rm -f "$IMG_FILE"

echo "[2/4] 빈 FAT32 이미지 생성 (${IMG_SIZE_MB}MB)"
dd if=/dev/zero of="$IMG_FILE" bs=1M count=$IMG_SIZE_MB status=none
mkfs.vfat -n "ESP" "$IMG_FILE" > /dev/null

echo "[3/4] 이미지 마운트 및 파일 복사"
sudo mkdir -p "$MOUNT_POINT"
sudo mount -o loop "$IMG_FILE" "$MOUNT_POINT"
trap 'sudo umount "$MOUNT_POINT" 2>/dev/null || true' EXIT
sudo cp -rT "$ESP_DIR" "$MOUNT_POINT"
sync

echo "[4/4] QEMU 실행"
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly=on,file=OVMF/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=OVMF/OVMF_VARS.fd \
  -drive format=raw,file="$IMG_FILE" \
  -m 512M \
  -serial mon:stdio \
  -net none \
  -no-reboot \
  -no-shutdown
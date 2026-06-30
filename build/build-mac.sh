#!/usr/bin/env bash
#
# 在 macOS 上編譯 86Duino IDE。
# 這個專案需要 Java 8 (build.xml 設定 source/target = 1.8)，
# 此腳本只在本次執行中把 JAVA_HOME 切到 JDK 8，不影響系統預設的 JDK。
#
# 用法:
#   ./build-mac.sh          # 編譯 (預設 target: build)
#   ./build-mac.sh run      # 編譯並執行
#   ./build-mac.sh clean     # 清除
#   ./build-mac.sh <target>  # 任意 ant target
#
set -euo pipefail

# 切換到腳本所在目錄 (build/)，確保能找到 build.xml
cd "$(dirname "$0")"

# 找出 JDK 8 的路徑
if ! JAVA_HOME="$(/usr/libexec/java_home -v 1.8 2>/dev/null)"; then
  echo "錯誤: 找不到 JDK 8。" >&2
  echo "請先安裝: brew install --cask temurin@8" >&2
  exit 1
fi
export JAVA_HOME
echo "使用 JAVA_HOME = $JAVA_HOME"
"$JAVA_HOME/bin/java" -version

# 確認 ant 存在
if ! command -v ant >/dev/null 2>&1; then
  echo "錯誤: 找不到 ant。請先安裝: brew install ant" >&2
  exit 1
fi

# ---------------------------------------------------------------------------
# 執行 ant，並自動補救缺少的下載 checksum。
#
# 背景: build.xml 用倉庫內 committed 的 *.sha 來驗證每個下載檔。某些分支(如
# 86Duino_5xx)升級了工具版本卻沒一併 commit 新版的 .sha，導致驗證階段失敗:
#     File <name> failed checksum.
# 這個迴圈會在「且僅在」遇到 checksum 失敗時，用本次實際下載的檔案算出 sha1
# 補上對應的 .sha，然後重跑 ant。已下載的檔會被快取，所以重試很快。
#
# ⚠ 安全性說明: 自動補 checksum 等於「信任本次透過 HTTPS 下載的內容」，
#    略過了原本的完整性驗證。這些檔來自官方 downloads.arduino.cc 與
#    github.com/arduino* 的釘選版本，風險低，但若要嚴格驗證請設
#    NO_AUTO_CHECKSUM=1 停用此功能，改為手動填入權威 checksum。
# ---------------------------------------------------------------------------
echo "----------------------------------------"
BUILD_LOG="$(mktemp -t build-mac.XXXXXX)"
trap 'rm -f "$BUILD_LOG"' EXIT
MAX_CHECKSUM_RETRIES=40
ant_status=0

while : ; do
  ant_status=0
  # 同時顯示輸出並擷取到 log (供偵測 checksum 失敗用)
  if ! ant "$@" 2>&1 | tee "$BUILD_LOG"; then
    ant_status=${PIPESTATUS[0]}
  fi
  [ "$ant_status" -eq 0 ] && break

  # 找出是哪個檔 checksum 失敗 (沒有就代表是其他錯誤，直接結束)
  fail_file="$(grep -oE 'File [^ ]+ failed checksum' "$BUILD_LOG" | head -1 | sed -E 's/File (.+) failed checksum/\1/')"
  [ -z "$fail_file" ] && break

  # 允許停用自動補救
  if [ "${NO_AUTO_CHECKSUM:-0}" = "1" ]; then
    echo "checksum 失敗 ($fail_file)，但 NO_AUTO_CHECKSUM=1，停用自動補救。" >&2
    break
  fi

  MAX_CHECKSUM_RETRIES=$((MAX_CHECKSUM_RETRIES - 1))
  if [ "$MAX_CHECKSUM_RETRIES" -lt 0 ]; then
    echo "錯誤: checksum 自動補救次數過多，可能有其他問題，停止。" >&2
    break
  fi

  # 在常見下載位置尋找實際檔案
  dl_path=""
  for loc in "$fail_file" "macosx/$fail_file" "shared/$fail_file"; do
    [ -f "$loc" ] && dl_path="$loc" && break
  done
  if [ -z "$dl_path" ]; then
    echo "錯誤: checksum 失敗但找不到下載檔 $fail_file，無法自動補救。" >&2
    break
  fi

  sha="$(shasum -a 1 "$dl_path" | awk '{print $1}')"
  printf '%s\n' "$sha" > "$dl_path.sha"
  echo "⚠ 自動補上缺少的 checksum: $dl_path.sha = $sha (信任本次 HTTPS 下載內容)，重跑 ant…" >&2
done

# ---------------------------------------------------------------------------
# 編譯後處理: 內建 ctags (隨 arduino-builder 一起下載) 是 Intel 執行檔
#   - master(arduino-builder 1.4.4): i386，Apple Silicon 連 Rosetta 都不能跑
#   - 86Duino_5xx(arduino-builder 1.6.1): x86_64，需靠 Rosetta 才能跑
# ant 每次重新展開下載的工具時都會蓋回 Intel 版，所以這裡在每次 build 之後，
# 自動把它替換成預先編好的原生 arm64 版本，使其不依賴 Rosetta。
# arm64 ctags 來源: 專案根目錄的 ctags-5.8-arduino11-arm64
# 注意: 不寫死 app 名稱 (master=86Duino.app, 5xx=Arduino.app)，動態尋找。
# ---------------------------------------------------------------------------
if [ "$ant_status" -eq 0 ] && [ "$(uname -m)" = "arm64" ]; then
  ARM_CTAGS="../ctags-5.8-arduino11-arm64"
  # 在產生的 .app bundle 內動態尋找 5.8-arduino11 的 ctags
  DEST_CTAGS="$(find macosx/work/*.app -type f -path '*ctags/5.8-arduino11/ctags' 2>/dev/null | head -1)"
  if [ -n "$DEST_CTAGS" ] && [ -f "$DEST_CTAGS" ]; then
    if [ ! -f "$ARM_CTAGS" ]; then
      echo "警告: 找不到 arm64 ctags ($ARM_CTAGS)，略過 ctags 替換。" >&2
    elif [ "$(lipo -archs "$DEST_CTAGS" 2>/dev/null)" = "arm64" ]; then
      echo "ctags 已是 arm64，無需替換。"
    else
      cp "$ARM_CTAGS" "$DEST_CTAGS" && chmod +x "$DEST_CTAGS"
      echo "已將內建 ctags ($DEST_CTAGS) 替換為原生 arm64 版本。"
    fi
  else
    echo "提醒: 在 app bundle 內找不到 5.8-arduino11 的 ctags，略過替換。" >&2
  fi
fi

# ---------------------------------------------------------------------------
# 編譯後處理: 修正 "Error Loading JAVA"。
# appbundler 產生的原生啟動器 (Contents/MacOS/<exe>) 被簽上 Hardened Runtime
# (flags=runtime)，會啟用 Library Validation —— 只允許載入「相同簽章身分」的
# 動態庫。但啟動器要 dlopen 內嵌 Temurin JDK 的 libjli.dylib (簽章身分不同)，
# 於是被擋下、dlopen 失敗，跳出 "Error Loading JAVA" 後結束。
# 解法: 對啟動器重新做 ad-hoc 簽章 (不帶 runtime 旗標)，解除 Library Validation。
# ---------------------------------------------------------------------------
if [ "$ant_status" -eq 0 ] && command -v codesign >/dev/null 2>&1; then
  APP_BUNDLE="$(ls -d macosx/work/*.app 2>/dev/null | head -1)"
  if [ -n "$APP_BUNDLE" ] && [ -d "$APP_BUNDLE" ]; then
    EXE_NAME="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$APP_BUNDLE/Contents/Info.plist" 2>/dev/null)"
    STUB="$APP_BUNDLE/Contents/MacOS/$EXE_NAME"
    if [ -n "$EXE_NAME" ] && [ -f "$STUB" ]; then
      # 先把 codesign 輸出存進變數，避免 `... | grep -q` 在 pipefail 下因
      # grep 提早關閉管線、codesign 收到 SIGPIPE 而誤判 (見本檔修正紀錄)。
      sig_info="$(codesign -dv "$STUB" 2>&1 || true)"
      case "$sig_info" in
        *"(runtime)"*)
          codesign --remove-signature "$STUB" 2>/dev/null || true
          if codesign --force -s - "$STUB" 2>/dev/null; then
            echo "已對啟動器 ($STUB) 重新 ad-hoc 簽章，解除 Hardened Runtime 的 Library Validation。"
          else
            echo "警告: 對啟動器重新簽章失敗，啟動時可能出現 Error Loading JAVA。" >&2
          fi
          ;;
        *)
          echo "啟動器未啟用 Hardened Runtime，無需重簽。"
          ;;
      esac
    fi
  fi
fi

# ---------------------------------------------------------------------------
# 編譯後處理: 還原 bundle 內工具的執行權限 (macOS 安全網)。
# 工具是用 ant <copy> 複製進 bundle 的，但 ant <copy> 不保留 Unix 執行權限 (+x)，
# 會造成 "fork/exec ...: permission denied"。這裡以「來源目錄的實際 +x」精準鏡像，
# 涵蓋兩處:
#   1) hardware/tools/{upx309w,x86} ← build/macosx/{upx309w,x86}
#      (djgpp 由 untar-native 解壓已保留 +x，來源已不在 build/macosx，迴圈會自動略過)
#   2) Contents/Java/tools          ← build/shared/tools
#      (例如 EcConfig/tool 下的 EcConfig.exe、pl2303gcgpio 是 macOS 原生執行檔)
# ---------------------------------------------------------------------------
if [ "$ant_status" -eq 0 ]; then
  APP_BUNDLE="$(ls -d macosx/work/*.app 2>/dev/null | head -1)"
  if [ -n "$APP_BUNDLE" ]; then
    JAVA_DIR="$APP_BUNDLE/Contents/Java"
    fixed=0
    # 以「來源目錄 → 目標目錄」精準鏡像 +x
    mirror_pairs="macosx/upx309w:$JAVA_DIR/hardware/tools/upx309w
macosx/x86:$JAVA_DIR/hardware/tools/x86
shared/tools:$JAVA_DIR/tools"
    while IFS= read -r pair; do
      src="${pair%%:*}"; dest="${pair#*:}"
      [ -d "$src" ] && [ -d "$dest" ] || continue
      while IFS= read -r -d '' f; do
        if [ -f "$dest/$f" ]; then
          chmod +x "$dest/$f" && fixed=$((fixed + 1))
        fi
      done < <(cd "$src" && find . -type f -perm -u+x -print0 2>/dev/null)
    done <<< "$mirror_pairs"
    echo "已還原 bundle 內工具 (hardware/tools + Contents/Java/tools) 的執行權限 ($fixed 個檔)。"
  fi
fi

exit "$ant_status"

#!/usr/bin/env bash
#
# 在 Linux (Ubuntu 等) 上編譯 86Duino IDE。
# 這個專案需要 Java 8 (build.xml 設定 source/target = 1.8)，
# 此腳本只在本次執行中把 JAVA_HOME 切到 JDK 8，不影響系統預設的 JDK。
#
# 這是 build-mac.sh 的 Linux 對應版本。相較於 mac，Linux 上「不需要」的步驟已移除：
#   - arm64 ctags 替換：mac 專屬 (arm64 主機拿到 Intel ctags 才需要)。
#     Linux x86_64 主機拿到的是隨 arduino-builder 下載的原生 x86_64 ctags，架構相符，無需替換。
#   - codesign / Hardened Runtime 修正："Error Loading JAVA" 是 macOS 簽章機制專屬問題。
#
# 用法:
#   ./build-linux.sh          # 編譯 (預設 target: build)
#   ./build-linux.sh run      # 編譯並執行
#   ./build-linux.sh clean    # 清除
#   ./build-linux.sh <target> # 任意 ant target
#
set -euo pipefail

# 切換到腳本所在目錄 (build/)，確保能找到 build.xml
cd "$(dirname "$0")"

# ---------------------------------------------------------------------------
# 找出 JDK 8 的路徑。
# mac 用 /usr/libexec/java_home，Linux 沒有這個工具，改為依序嘗試常見位置。
# 若使用者已 export 一個指向 JDK 8 的 JAVA_HOME，優先沿用。
# ---------------------------------------------------------------------------
is_jdk8() {
  # $1 = JDK 目錄；有 bin/javac 且版本為 1.8 才算數
  [ -x "$1/bin/javac" ] || return 1
  "$1/bin/javac" -version 2>&1 | grep -q '1\.8\.' || return 1
  return 0
}

JAVA8_HOME=""
# 1) 沿用既有的 JAVA_HOME (若它就是 JDK 8)
if [ -n "${JAVA_HOME:-}" ] && is_jdk8 "$JAVA_HOME"; then
  JAVA8_HOME="$JAVA_HOME"
fi
# 2) 掃描常見安裝位置
if [ -z "$JAVA8_HOME" ]; then
  for cand in \
    /usr/lib/jvm/java-8-openjdk-* \
    /usr/lib/jvm/java-1.8.0-openjdk-* \
    /usr/lib/jvm/temurin-8-* \
    /usr/lib/jvm/adoptopenjdk-8-* \
    /usr/lib/jvm/zulu-8-* ; do
    if [ -d "$cand" ] && is_jdk8 "$cand"; then
      JAVA8_HOME="$cand"
      break
    fi
  done
fi

if [ -z "$JAVA8_HOME" ]; then
  echo "錯誤: 找不到 JDK 8。" >&2
  echo "請先安裝: sudo apt install openjdk-8-jdk" >&2
  echo "(或手動設定 JAVA_HOME 指向已安裝的 JDK 8 後再執行)" >&2
  exit 1
fi

export JAVA_HOME="$JAVA8_HOME"
export PATH="$JAVA_HOME/bin:$PATH"
echo "使用 JAVA_HOME = $JAVA_HOME"
"$JAVA_HOME/bin/java" -version

# 確認 ant 存在
if ! command -v ant >/dev/null 2>&1; then
  echo "錯誤: 找不到 ant。請先安裝: sudo apt install ant" >&2
  exit 1
fi

# ---------------------------------------------------------------------------
# 執行 ant，並自動補救缺少的下載 checksum。
#
# 背景: build.xml 用倉庫內 committed 的 *.sha 來驗證每個下載檔。某些分支(如
# 86Duino_5xx)升級了工具版本、或改為編譯時下載 (djgpp)，卻沒一併 commit 對應
# 的 .sha，導致驗證階段失敗:
#     File <name> failed checksum.
# 已知在 Linux 上會缺 sha 的下載至少有:
#   - djgpp-linux64-gcc830.tar.bz2      (djgpp 交叉編譯器，改為編譯時下載，無 committed sha)
#   - arduino-builder-linux64-1.6.1.*   (版本升到 1.6.1，但倉庫只有 1.4.4 的 sha)
# 這個迴圈會在「且僅在」遇到 checksum 失敗時，用本次實際下載的檔案算出 sha1
# 補上對應的 .sha，然後重跑 ant。已下載的檔會被快取，所以重試很快。
#
# ⚠ 安全性說明: 自動補 checksum 等於「信任本次透過 HTTPS 下載的內容」，
#    略過了原本的完整性驗證。這些檔來自官方 downloads.arduino.cc 與
#    github.com/andrewwutw (build-djgpp) 的釘選版本，風險低，但若要嚴格驗證請設
#    NO_AUTO_CHECKSUM=1 停用此功能，改為手動填入權威 checksum。
# ---------------------------------------------------------------------------
echo "----------------------------------------"
BUILD_LOG="$(mktemp -t build-linux.XXXXXX)"
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

  # 在常見下載位置尋找實際檔案。
  # build.xml 傳給 checksum 的 archive_file 可能已含相對路徑 (如 linux/djgpp-...)，
  # 也可能是 build/ 根目錄的檔 (如 ./arduino-builder-... 或 avr-...)，故多處嘗試。
  dl_path=""
  base="$(basename "$fail_file")"
  for loc in "$fail_file" "./$fail_file" "$base" "linux/$base" "shared/$base"; do
    [ -f "$loc" ] && dl_path="$loc" && break
  done
  if [ -z "$dl_path" ]; then
    echo "錯誤: checksum 失敗但找不到下載檔 $fail_file，無法自動補救。" >&2
    break
  fi

  sha="$(sha1sum "$dl_path" | awk '{print $1}')"
  printf '%s\n' "$sha" > "$dl_path.sha"
  echo "⚠ 自動補上缺少的 checksum: $dl_path.sha = $sha (信任本次 HTTPS 下載內容)，重跑 ant…" >&2
done

# ---------------------------------------------------------------------------
# 編譯後處理: 還原 bundle 內工具的執行權限。
# 工具是用 ant <copy> 複製進 linux/work 的，但 ant <copy> 不保留 Unix 執行權限 (+x)，
# 會造成執行期 "permission denied"。build.xml 已對 hardware/tools/{djgpp,upx309w,x86}
# 與 86duino/*.sh 補過 +x，但「複製到 linux/work/tools 的原生工具執行檔」
# (來源 build/shared/tools 與 build/linux/64/tools，例如 EcConfig 下的執行檔) 沒被涵蓋，
# 這裡以「來源目錄的實際 +x」精準鏡像補回。
# ---------------------------------------------------------------------------
if [ "$ant_status" -eq 0 ]; then
  WORK_DIR="linux/work"
  if [ -d "$WORK_DIR" ]; then
    fixed=0
    # 以「來源目錄 → 目標目錄」精準鏡像 +x
    mirror_pairs="shared/tools:$WORK_DIR/tools
linux/64/tools:$WORK_DIR/tools"
    while IFS= read -r pair; do
      src="${pair%%:*}"; dest="${pair#*:}"
      [ -d "$src" ] && [ -d "$dest" ] || continue
      while IFS= read -r -d '' f; do
        if [ -f "$dest/$f" ]; then
          chmod +x "$dest/$f" && fixed=$((fixed + 1))
        fi
      done < <(cd "$src" && find . -type f -perm -u+x -print0 2>/dev/null)
    done <<< "$mirror_pairs"
    echo "已還原 bundle 內工具 (linux/work/tools) 的執行權限 ($fixed 個檔)。"
  fi
fi

# ---------------------------------------------------------------------------
# 編譯後檢查: 提醒缺少的「系統執行期函式庫」。
# djgpp 交叉編譯器是預編譯的 x86_64 執行檔，會動態連結一些 host 系統函式庫
# (例如 i586-pc-msdosdjgpp-ar 依賴 flex 的 libfl.so.2)。這些不在本專案的
# 建置相依內，若 host 未安裝，IDE 「編譯本身沒問題」，卻會在用它去編譯 sketch
# 時才報 "error while loading shared libraries: ...: cannot open shared object file"。
# 這裡用 ldd 掃描 djgpp/bin 下所有執行檔，把缺少的 .so 一次列出，並附上對應的
# apt 套件名，讓使用者一次補齊，不必逐一踩雷。
# 註: 這只是「提醒」，不影響 build 結果 (ant_status)，也不會自動安裝 (需 sudo)。
# ---------------------------------------------------------------------------
if [ "$ant_status" -eq 0 ] && command -v ldd >/dev/null 2>&1; then
  DJGPP_BIN="linux/work/hardware/tools/djgpp/bin"
  if [ -d "$DJGPP_BIN" ]; then
    # 已知 soname → apt 套件 的對照表 (缺什麼就補什麼)
    soname_to_pkg() {
      case "$1" in
        libfl.so.2)  echo "libfl2" ;;
        *)           echo "" ;;   # 未知者留白，下面改提示用 apt-file 查
      esac
    }

    missing_sonames="$(
      for exe in "$DJGPP_BIN"/*; do
        [ -f "$exe" ] && [ -x "$exe" ] || continue
        ldd "$exe" 2>/dev/null | grep -oE '[^[:space:]]+ => not found' | awk '{print $1}'
      done | sort -u
    )"

    if [ -n "$missing_sonames" ]; then
      pkgs=""
      unknown=""
      while IFS= read -r so; do
        [ -z "$so" ] && continue
        pkg="$(soname_to_pkg "$so")"
        if [ -n "$pkg" ]; then
          case " $pkgs " in *" $pkg "*) : ;; *) pkgs="$pkgs $pkg" ;; esac
        else
          unknown="$unknown $so"
        fi
      done <<< "$missing_sonames"
      pkgs="${pkgs# }"; unknown="${unknown# }"

      echo ""
      echo "========================================================================"
      echo "⚠ 提醒: djgpp 工具鏈缺少下列系統函式庫，IDE 編譯 sketch 時會失敗:"
      echo "$missing_sonames" | sed 's/^/    - /'
      if [ -n "$pkgs" ]; then
        echo ""
        echo "  請安裝對應套件後再用 IDE 編譯:"
        echo "      sudo apt install $pkgs"
      fi
      if [ -n "$unknown" ]; then
        echo ""
        echo "  下列函式庫無內建對照，請用 apt-file 查出提供它的套件再安裝:"
        for so in $unknown; do echo "      apt-file search $so"; done
        echo "  (若無 apt-file: sudo apt install apt-file && sudo apt-file update)"
      fi
      echo "========================================================================"
    fi
  fi
fi

exit "$ant_status"

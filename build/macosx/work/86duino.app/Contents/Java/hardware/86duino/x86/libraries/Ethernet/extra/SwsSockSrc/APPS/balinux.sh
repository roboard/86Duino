#!/bin/bash

if [ -z "$VERBOSITY" ]; then
export VERBOSITY=0
fi
if [ -z "$AUTODEPENDS" ]; then
export AUTODEPENDS=0
fi

if [ -z "$MAKEFLAGS" ]; then
export MAKEFLAGS="-s -k"
fi
export MAKE="make"

: ${debug:="0"}
: ${shared:="0"}

lib="../LIBS"
if [ "$host" != "0" ]; then
  bin="../bin"
  mkdir -p "$bin"
  if [ "$shared" != "0" ]; then
    binsws="../bin-sws"
    mkdir -p "$binsws"
  fi
fi

if [ -x "/usr/bin/i586-pc-msdosdjgpp-gcc" -a "$djgpp" != "0" ]; then
  libdj="$lib/DJGPP"
  bindos="../bin-dos"
  mkdir -p "$bindos"
fi

if [ -x "/usr/bin/i586-mingw32msvc-gcc" -a "$mingw" != "0" ]; then
  libw32="$lib/mingw"
  binw32="../bin-w32"
  mkdir -p "$binw32"
  if [ "$shared" != "0" ]; then
    binw32sws="../bin-w32-sws"
    mkdir -p "$binw32sws"
  fi
fi

if [ "$clean" != "0" ]; then
  echo "cleaning libs..."
  # Remove all libs
  $MAKE -C "$lib/linux" clean
  $MAKE -C "$lib/linux" DEBUG=1 clean
  $MAKE -C "$lib/linux" SHARED=1 clean
  $MAKE -C "$lib/linux" SHARED=1 DEBUG=1 clean
  if [ -n "$libdj" -a -d "$libdj" ]; then
    $MAKE -C "$libdj" XDJGPP=1 clean
    $MAKE -C "$libdj" XDJGPP=1 DEBUG=1 clean
  fi
  if [ -n "$libw32" -a -d "$libw32" ]; then
    $MAKE -C "$libw32" XMINGW32=1 clean
    $MAKE -C "$libw32" XMINGW32=1 DEBUG=1 clean
    $MAKE -C "$libw32" XMINGW32=1 SHARED=1 clean
    $MAKE -C "$libw32" XMINGW32=1 SHARED=1 DEBUG=1 clean
  fi
fi

if [ -n "$1" ]; then
  apps=$@
else
  apps=*
fi

for app in $apps ; do
  dir="$app/gnu"
  if [ -d "$dir" -a -e "$dir/makefile" ]; then

    if [ -n "$bin" ]; then
      if [ "$debug" != "0" ]; then
        echo "Building $app-DEBUG-C++..."
        $MAKE -C "$dir" DEBUG=cxx clean all
        $MAKE -C "$dir" DEBUG=cxx clean
      fi

      echo "Building $app..."
      $MAKE -C "$dir" clean all && mv -f "$dir/$app" "$bin"
      $MAKE -C "$dir" clean
    fi

    if [ -n "$binsws" ]; then
      if [ "$debug" != "0" ]; then
        echo "Building $app-DEBUG-C++-so"
        $MAKE -C "$dir" DEBUG=cxx SHARED=1 SWSSOCK=1 clean all
        $MAKE -C "$dir" DEBUG=cxx SHARED=1 SWSSOCK=1 clean
      fi

      echo "Building $app-so"
      $MAKE -C "$dir" SHARED=1 SWSSOCK=1 clean all && mv -f "$dir/$app" "$binsws"
      $MAKE -C "$dir" SHARED=1 SWSSOCK=1 clean
    fi

    if [ -n "$bindos" ]; then
      if [ "$debug" != "0" ]; then
        echo "Building $app-DEBUG-C++ with DJGPP..."
        $MAKE -C "$dir" DEBUG=cxx XDJGPP=1 clean all
        $MAKE -C "$dir" DEBUG=cxx XDJGPP=1 clean
      fi

      echo "Building $app with DJGPP..."
      $MAKE -C "$dir" XDJGPP=1 clean all && mv -f $dir/*.exe "$bindos"
      $MAKE -C "$dir" XDJGPP=1 clean
    fi

    if [ -n "$binw32" ]; then
      if [ "$debug" != "0" ]; then
        echo "Building $app-DEBUG-C++ with MINGW32..."
        $MAKE -C "$dir" DEBUG=cxx XMINGW32=1 clean all
        $MAKE -C "$dir" DEBUG=cxx XMINGW32=1 clean
      fi

      echo "Building $app with MINGW32..."
      $MAKE -C "$dir" XMINGW32=1 clean all && mv -f $dir/*.exe "$binw32"
      $MAKE -C "$dir" XMINGW32=1 clean
    fi

    if [ -n "$binw32sws" ]; then
      if [ "$debug" != "0" ]; then
        echo "Building $app-DEBUG-C++-DLL with MINGW32..."
        $MAKE -C "$dir" DEBUG=cxx XMINGW32=1 SHARED=1 SWSSOCK=1 clean all
        $MAKE -C "$dir" DEBUG=cxx XMINGW32=1 SHARED=1 SWSSOCK=1 clean
      fi

      echo "Building $app-DLL with MINGW32..."
      $MAKE -C "$dir" XMINGW32=1 SHARED=1 SWSSOCK=1 clean all && mv -f $dir/*.exe "$binw32sws"
      $MAKE -C "$dir" XMINGW32=1 SHARED=1 SWSSOCK=1 clean
    fi
  fi
done

# Save so's
if [ -n "$binsws" -a -d "$lib/linux" ]; then
  for so in $lib/linux/*.so ; do
    cp -f "$so" "$binsws"
  done
fi

# Save dll's
if [ -n "$binw32sws" -a -d "$libw32" ]; then
  for dll in $libw32/*.dll ; do
    cp -f "$dll" "$binw32sws"
  done
fi

if [ "$clean" != "0" ]; then
  echo "cleaning libs..."
  # Cleanup libs
  $MAKE -C "$lib/linux" clean
  $MAKE -C "$lib/linux" DEBUG=1 clean
  $MAKE -C "$lib/linux" SHARED=1 clean
  $MAKE -C "$lib/linux" SHARED=1 DEBUG=1 clean
  if [ -n "$libdj" -a -d "$libdj" ]; then
    $MAKE -C "$libdj" XDJGPP=1 clean
    $MAKE -C "$libdj" XDJGPP=1 DEBUG=1 clean
  fi
  if [ -n "$libw32" -a -d "$libw32" ]; then
    $MAKE -C "$libw32" XMINGW32=1 SHARED=1 clean
    $MAKE -C "$libw32" XMINGW32=1 SHARED=1 DEBUG=1 clean
    $MAKE -C "$libw32" XMINGW32=1 clean
    $MAKE -C "$libw32" XMINGW32=1 DEBUG=1 clean
  fi
fi

# Remove test apps
for app in iptest p2ptest udptest ; do
  if [ -n "$bin" -a -e "$bin/$app" ]; then
    rm -f "$bin/$app"
  fi
  if [ -n "$binsws" -a -e "$binsws/$app" ]; then
    rm -f "$binsws/$app"
  fi
  if [ -n "$bindos" -a -e "$bindos/$app.exe" ]; then
    rm -f "$bindos/${app}.exe"
  fi
  if [ -n "$binw32" -a -e "$binw32/$app.exe" ]; then
    rm -f "$binw32/$app.exe"
  fi
  if [ -n "$binw32sws" -a -e "$binw32sws/$app.exe" ]; then
    rm -f "$binw32sws/$app.exe"
  fi
done

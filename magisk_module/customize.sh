SKIPUNZIP=0

if [ -e $MODPATH/bin/$ARCH/sudo ]; then
    mkdir -p $MODPATH/system/bin
    mv $MODPATH/bin/$ARCH/sudo $MODPATH/system/bin/sudo
    rm -rf $MODPATH/bin
else
    abort "! Unsupport Arch: $ARCH"
fi

# Set permissions
ui_print "- Setting permissions"
set_perm $MODPATH/system/bin/sudo 0 0 0755

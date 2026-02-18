# Make dummy AGL framework to get around Qt 6.7.3 build errors

sudo mkdir -p /Library/Frameworks/AGL.framework/Versions/A
echo "void _agl_stub(void){}" > /tmp/agl_stub.c
sudo clang -dynamiclib -o /Library/Frameworks/AGL.framework/Versions/A/AGL /tmp/agl_stub.c -install_name /System/Library/Frameworks/AGL.framework/Versions/A/AGL
cd /Library/Frameworks/AGL.framework
sudo ln -sf Versions/A/AGL AGL
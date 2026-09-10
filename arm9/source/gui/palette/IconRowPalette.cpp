#include "common.h"
#include "IconRowPalette.h"

u32 IconRowPalette::GetHashCode() const
{
    u32 hashcode = 2748019;
    hashcode = hashcode * 7302013 + _topColor.r;
    hashcode = hashcode * 7302013 + _topColor.g;
    hashcode = hashcode * 7302013 + _topColor.b;
    hashcode = hashcode * 7302013 + _bottomColor.r;
    hashcode = hashcode * 7302013 + _bottomColor.g;
    hashcode = hashcode * 7302013 + _bottomColor.b;
    return hashcode;
}

#include "common.h"
#include "IconButtonSelectorPalette.h"

u32 IconButtonSelectorPalette::GetHashCode() const
{
    u32 hashcode = 5821337;
    hashcode = hashcode * 7302013 + _backdropColor.r;
    hashcode = hashcode * 7302013 + _backdropColor.g;
    hashcode = hashcode * 7302013 + _backdropColor.b;
    hashcode = hashcode * 7302013 + _fillColor.r;
    hashcode = hashcode * 7302013 + _fillColor.g;
    hashcode = hashcode * 7302013 + _fillColor.b;
    hashcode = hashcode * 7302013 + _borderColor.r;
    hashcode = hashcode * 7302013 + _borderColor.g;
    hashcode = hashcode * 7302013 + _borderColor.b;
    return hashcode;
}

/*Copyright (c) 2018 Jonathan Campbell

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#pragma once
#include <stdint.h> //needed for uint_fast8_t
#include "Bitfield.h"

namespace WUIF {
    namespace FLAGS {
        BIT_FIELD(uint_fast8_t, GFX_Flags);
        BIT_MASK(GFX_Flags, WARP,  1);
        BIT_MASK(GFX_Flags, D2D,   2);
        BIT_MASK(GFX_Flags, D3D11, 3);
        BIT_MASK(GFX_Flags, D3D12, 4);
    }

    //make sure to add to OSCheck in OSCheck.h
    enum class OSVersion {
        UNKNOWN = 0,
        WIN7,
        WIN8,
        WIN8_1,
        WIN10,
        WIN10_1511, //build 10586 - November Update
        WIN10_1607, //build 14393 - Anniversary Update
        WIN10_1703, //build 15063 - Creators Update
        WIN10_1709, //build 16299 - Fall Creators Update
        WIN10_1803, //build 17134 - April 2018 Update
        WIN10_1809, //build 17763 - October 2018 Update
    };
}
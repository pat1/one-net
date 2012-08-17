// one_net_display.h
#include <fstream>
#include "string_utils.h"
#include "one_net_types.h"
#include "one_net_application.h"
#include "on_display.h"


void display_flags_byte(UInt8 flags, std::ostream& outs)
{
    outs << "Settings : 0x";
    outs << hex << (int) flags << " (Bits = " << value_to_bit_string(flags, 8)
         << ")";

    outs << " -- JOINED : ";
    outs << ((flags & ON_JOINED) ? "true" : "false");
    outs << " -- SEND TO MASTER : ";
    outs << ((flags & ON_SEND_TO_MASTER) ? "true" : "false");
    outs << " -- Reject Invalid Msg. ID : ";
    outs << ((flags & ON_REJECT_INVALID_MSG_ID) ? "true" : "false");
    outs << "\n";
    outs <<  " -- Block/Stream Elevate Data Rate : ";
    outs << ((flags & ON_BS_ELEVATE_DATA_RATE) ? "true" : "false");
    outs <<  " -- Block/Stream Change Channel : ";
    outs << ((flags & ON_BS_CHANGE_CHANNEL) ? "true" : "false");
    outs <<  " -- Block/Stream High Priority : ";
    outs << ((flags & ON_BS_HIGH_PRIORITY) ? "true" : "false");
    outs <<  " -- Block/Stream Allowed : ";
    outs << ((flags & ON_BS_ALLOWED) ? "true" : "false");
}

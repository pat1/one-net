// calculate_flags_byte.cpp
#include <iostream>
#include <cstdio>
#include "one_net_application.h"
using namespace std;


const string FLAG_BIT_QUESTIONS[7] =
{
    "Is the device in the network?",
    "Should the device send status updates to the Master?",
    "Should the device reject invalid Message IDs?",
    "Should the device elevate data rates during long block / stream transfers?",
    "Should the device change channels during long block / stream transfers?",
    "Should the device send high priority messages during long block / stream transfers?",
    "Are long block / stream messages allowed for this device?"
};


const UInt8 BIT_MASKS[7] =
{
    ON_JOINED,
    ON_SEND_TO_MASTER,
    ON_REJECT_INVALID_MSG_ID,
    ON_BS_ELEVATE_DATA_RATE,
    ON_BS_CHANGE_CHANNEL,
    ON_BS_HIGH_PRIORITY,
    ON_BS_ALLOWED
};


bool get_response(string question)
{
    cout << question << "  ";
    string answer;
    getline(cin, answer, '\n');
    if(answer == "Y" || answer == "y")
    {
        return true;
    }
    else if(answer == "N" || answer == "n")
    {
        return false;
    }
    cout << "Please answer 'Y' or 'N' for all questions.\n";
    return get_response(question);
}


int main()
{
    cout << "Please answer 'Y' or 'N' for all questions.\n";

    UInt8 flags = 0;
    for(UInt8 i = 0;i < 7; i++)
    {
        if(get_response(FLAG_BIT_QUESTIONS[i]))
        {
            flags |= BIT_MASKS[i];
        }
    }

    printf("Flags Byte: 0x%02X\n", flags);
    return 0;
}

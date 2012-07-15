#ifndef ATTRIBUTE_H
#define	ATTRIBUTE_H


#include <fstream>
#include <vector>
#include <string>
using namespace std;


class attribute
{
public:
    enum ATTRIBUTE
    {
        ATTRIBUTE_TIMESTAMP,
        ATTRIBUTE_ENCODED_BYTES,
        ATTRIBUTE_DECODED_BYTES,
        ATTRIBUTE_ENCODED_PAYLOAD,
        ATTRIBUTE_ENCRYPTED_PAYLOAD,
        ATTRIBUTE_DECRYPTED_PAYLOAD,
        ATTRIBUTE_PAYLOAD_DETAIL,
        ATTRIBUTE_HEADER,
        ATTRIBUTE_RPTR_DID,
        ATTRIBUTE_SRC_DID,
        ATTRIBUTE_DST_DID,
        ATTRIBUTE_NID,
        ATTRIBUTE_PID,
        ATTRIBUTE_MSG_ID,
        ATTRIBUTE_MSG_CRC,
        ATTRIBUTE_PAYLOAD_CRC,
        ATTRIBUTE_HOPS,
        ATTRIBUTE_KEY,
        ATTRIBUTE_VALID_PKT,

        // Application Payload Attributes
        ATTRIBUTE_SRC_UNIT,
        ATTRIBUTE_DST_UNIT,
        ATTRIBUTE_MSG_CLASS,
        ATTRIBUTE_MSG_TYPE,
        ATTRIBUTE_MSG_DATA,

        // Invite Payload Attributes
        ATTRIBUTE_INVITE_VERSION,
        ATTRIBUTE_INVITE_DID,
        ATTRIBUTE_INVITE_KEY,
        ATTRIBUTE_INVITE_FEATURES,



        ATTRIBUTE_INVALID // catch-all enumeration type for an invalid attribute
    };

    static const int NUM_ATTRIBUTES;
    static const string ATTRIBUTE_STR[];

    attribute();
    attribute(const attribute& orig);
    attribute& operator = (const attribute& that);
    ~attribute();
    void set_attributes(bool present);
    bool set_attribute(ATTRIBUTE att, bool present);
    bool set_attribute(const string& att_string, bool present);
    bool get_attribute(ATTRIBUTE att) const;
    ATTRIBUTE string_to_attribute(const string& att_string);
    static string attribute_to_string(ATTRIBUTE att, bool make_capital = false);
    void display_attributes(ostream& outs) const;
    bool display(ATTRIBUTE att) const;

private:
    vector<bool> attribute_present;
};


#endif	/* ATTRIBUTE_H */


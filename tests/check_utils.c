/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>

#include "ua_types.h"
#include "ua_client.h"
#include "ua_util.h"
#include "check.h"

START_TEST(EndpointUrl_split) {
    UA_String hostname = UA_STRING_NULL;
    UA_String path = UA_STRING_NULL;
    UA_UInt16 port = 0;

    // check for too short url
    UA_String endPointUrl = UA_STRING("inv.ali:/");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);

    // check for opc.tcp:// protocol
    endPointUrl = UA_STRING("inv.ali://");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);

    // empty url
    endPointUrl = UA_STRING("opc.tcp://");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);
    ck_assert(UA_String_equal(&hostname, &UA_STRING_NULL));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // only hostname
    endPointUrl = UA_STRING("opc.tcp://hostname");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path), UA_STATUSCODE_GOOD);
    UA_String expected = UA_STRING("hostname");
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // empty port
    endPointUrl = UA_STRING("opc.tcp://hostname:");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // specific port
    endPointUrl = UA_STRING("opc.tcp://hostname:1234");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_GOOD);
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 1234);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // IPv6
    endPointUrl = UA_STRING("opc.tcp://[2001:0db8:85a3::8a2e:0370:7334]:1234/path");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path), UA_STATUSCODE_GOOD);
    expected = UA_STRING("[2001:0db8:85a3::8a2e:0370:7334]");
    UA_String expectedPath = UA_STRING("path");
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 1234);
    ck_assert(UA_String_equal(&path, &expectedPath));

    // empty hostname
    endPointUrl = UA_STRING("opc.tcp://:");
    port = 0;
    path = UA_STRING_NULL;
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);
    ck_assert(UA_String_equal(&hostname, &UA_STRING_NULL));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // empty hostname and no port
    endPointUrl = UA_STRING("opc.tcp:///");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path), UA_STATUSCODE_GOOD);
    ck_assert(UA_String_equal(&hostname, &UA_STRING_NULL));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // overlength port
    endPointUrl = UA_STRING("opc.tcp://hostname:12345678");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);

    // port not a number
    endPointUrl = UA_STRING("opc.tcp://hostname:6x6");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path),
                      UA_STATUSCODE_BADTCPENDPOINTURLINVALID);
    expected = UA_STRING("hostname");
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // no port but path
    endPointUrl = UA_STRING("opc.tcp://hostname/");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path), UA_STATUSCODE_GOOD);
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 0);
    ck_assert(UA_String_equal(&path, &UA_STRING_NULL));

    // port and path
    endPointUrl = UA_STRING("opc.tcp://hostname:1234/path");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path), UA_STATUSCODE_GOOD);
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 1234);
    ck_assert(UA_String_equal(&path, &expectedPath));

    // port and path with a slash
    endPointUrl = UA_STRING("opc.tcp://hostname:1234/path/");
    ck_assert_uint_eq(UA_parseEndpointUrl(&endPointUrl, &hostname, &port, &path), UA_STATUSCODE_GOOD);
    ck_assert(UA_String_equal(&hostname, &expected));
    ck_assert_uint_eq(port, 1234);
    ck_assert(UA_String_equal(&path, &expectedPath));
}
END_TEST


START_TEST(readNumber) {
    UA_UInt32 result;
    ck_assert_uint_eq(UA_readNumber((UA_Byte*)"x", 1, &result), 0);

    ck_assert_uint_eq(UA_readNumber((UA_Byte*)"1x", 2, &result), 1);
    ck_assert_uint_eq(result, 1);

    ck_assert_uint_eq(UA_readNumber((UA_Byte*)"123456789", 9, &result), 9);
    ck_assert_uint_eq(result, 123456789);
}
END_TEST


START_TEST(StatusCode_msg) {
#ifndef UA_ENABLE_STATUSCODE_DESCRIPTIONS
    return;
#endif
        // first element in table
    ck_assert_str_eq(UA_StatusCode_name(UA_STATUSCODE_GOOD), "Good");

        // just some randomly picked status codes
    ck_assert_str_eq(UA_StatusCode_name(UA_STATUSCODE_BADNOCOMMUNICATION),
                     "BadNoCommunication");

    ck_assert_str_eq(UA_StatusCode_name(UA_STATUSCODE_GOODNODATA), "GoodNoData");

        // last element in table
    ck_assert_str_eq(UA_StatusCode_name(UA_STATUSCODE_BADMAXCONNECTIONSREACHED),
                     "BadMaxConnectionsReached");

        // an invalid status code
    ck_assert_str_eq(UA_StatusCode_name(0x80123456), "Unknown StatusCode");
}
END_TEST


static void assertNodeIdString(const UA_String *gotStr, const char* expectedStr) {
    size_t expectedStringLength = strlen(expectedStr);
    ck_assert_uint_ge(gotStr->length, expectedStringLength);
    char *gotChars = (char*)UA_malloc(gotStr->length+1);
    memcpy(gotChars, gotStr->data, gotStr->length);
    gotChars[gotStr->length] = 0;
    ck_assert_str_eq(gotChars, expectedStr);
    UA_free(gotChars);
}

START_TEST(idToStringNumeric) {
    UA_NodeId n;
    UA_String str = UA_STRING_NULL;

    n = UA_NODEID_NUMERIC(0,0);
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=0;i=0");

    n = UA_NODEID_NUMERIC(12345,1234567890);
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=12345;i=1234567890");

    n = UA_NODEID_NUMERIC(0xFFFF,0xFFFFFFFF);
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=65535;i=4294967295");

    UA_String_deleteMembers(&str);
} END_TEST

START_TEST(idToStringString) {
    UA_NodeId n;
    UA_String str = UA_STRING_NULL;

    n = UA_NODEID_STRING(0,"");
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=0;i=");

    n = UA_NODEID_STRING(54321,"Some String");
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=54321;i=Some String");

    UA_String_deleteMembers(&str);
} END_TEST

START_TEST(idToStringGuid) {
    UA_NodeId n;
    UA_String str = UA_STRING_NULL;

    UA_Guid g = UA_GUID_NULL;

    n = UA_NODEID_GUID(0,UA_GUID_NULL);
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=0;i=00000000-0000-0000-0000-000000000000");

    g.data1 = 0xA123456C;
    g.data2 = 0x0ABC;
    g.data3 = 0x1A2B;
    g.data4[0] = 0x81;
    g.data4[1] = 0x5F;
    g.data4[2] = 0x68;
    g.data4[3] = 0x72;
    g.data4[4] = 0x12;
    g.data4[5] = 0xAA;
    g.data4[6] = 0xEE;
    g.data4[7] = 0x1B;

    n = UA_NODEID_GUID(65535,g);
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=65535;i=a123456c-0abc-1a2b-815f-687212aaee1b");

    g.data1 = 0xFFFFFFFF;
    g.data2 = 0xFFFF;
    g.data3 = 0xFFFF;
    g.data4[0] = 0xFF;
    g.data4[1] = 0xFF;
    g.data4[2] = 0xFF;
    g.data4[3] = 0xFF;
    g.data4[4] = 0xFF;
    g.data4[5] = 0xFF;
    g.data4[6] = 0xFF;
    g.data4[7] = 0xFF;

    n = UA_NODEID_GUID(65535,g);
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=65535;i=ffffffff-ffff-ffff-ffff-ffffffffffff");

    UA_String_deleteMembers(&str);
} END_TEST

START_TEST(idToStringByte) {
    UA_NodeId n;
    UA_String str = UA_STRING_NULL;

    n.namespaceIndex = 0;
    n.identifierType = UA_NODEIDTYPE_BYTESTRING;
    n.identifier.byteString.data = NULL;
    n.identifier.byteString.length = 0;
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=0;i=");

    UA_ByteString bs = UA_BYTESTRING_NULL;

    bs.length = 1;
    bs.data = (UA_Byte*)UA_malloc(bs.length);
    bs.data[0] = 0x2C;
    n.identifier.byteString = bs;
    n.namespaceIndex = 123;
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=123;i=2c");
    UA_free(bs.data);

    bs.length = 5;
    bs.data = (UA_Byte*)UA_malloc(bs.length);
    bs.data[0] = 0x21;
    bs.data[1] = 0x83;
    bs.data[2] = 0xE0;
    bs.data[3] = 0x54;
    bs.data[4] = 0x78;
    n.identifier.byteString = bs;
    n.namespaceIndex = 599;
    UA_NodeId_toString(&n, &str);
    assertNodeIdString(&str, "ns=599;i=2183e05478");
    UA_free(bs.data);

    UA_String_deleteMembers(&str);
} END_TEST



static Suite* testSuite_Utils(void) {
    Suite *s = suite_create("Utils");
    TCase *tc_endpointUrl_split = tcase_create("EndpointUrl_split");
    tcase_add_test(tc_endpointUrl_split, EndpointUrl_split);
    suite_add_tcase(s,tc_endpointUrl_split);
    TCase *tc_utils = tcase_create("Utils");
    tcase_add_test(tc_utils, readNumber);
    tcase_add_test(tc_utils, StatusCode_msg);
    suite_add_tcase(s,tc_utils);


    TCase *tc1 = tcase_create("test nodeid string");
    tcase_add_test(tc1, idToStringNumeric);
    tcase_add_test(tc1, idToStringString);
    tcase_add_test(tc1, idToStringGuid);
    tcase_add_test(tc1, idToStringByte);
    suite_add_tcase(s, tc1);

    return s;
}

int main(void) {
    Suite *s = testSuite_Utils();
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr,CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

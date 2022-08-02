// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/testing/gtest/gtest.h>
#include <dots/serialization/CborSerializer.h>
#include <serialization/TestSerializer.h>

struct CborSerializerTestDataEncoded : SerializerTestDataEncoded<dots::serialization::CborSerializer>
{
    //
    // fundamental
    //

    data_t boolFalse{ 0xF4 };
    data_t boolTrue{ 0xF5 };

    data_t int8Zero{ 0x00 };
    data_t int8Positive{ 0x18, 0x2A };
    data_t int8Negative{ 0x38, 0x29 };

    data_t uint8Zero{ 0x00 };
    data_t uint8Positive1{ 0x18, 0x2A };
    data_t uint8Positive2{ 0x18, 0xAA };

    data_t int16Zero{ 0x00 };
    data_t int16Positive{ 0x19, 0x30, 0x39 };
    data_t int16Negative{ 0x39, 0x30, 0x38 };

    data_t uint16Zero{ 0x00 };
    data_t uint16Positive1{ 0x19, 0x30, 0x39 };
    data_t uint16Positive2{ 0x19, 0xB0, 0x39 };

    data_t int32Zero{ 0x00 };
    data_t int32Positive{ 0x1A, 0x00, 0xBC, 0x61, 0xBD };
    data_t int32Negative{ 0x3A, 0x00, 0xBC, 0x61, 0xBC };

    data_t uint32Zero{ 0x00 };
    data_t uint32Positive1{ 0x1A, 0x00, 0xBC, 0x61, 0xBD };
    data_t uint32Positive2{ 0x1A, 0x80, 0xBC, 0x61, 0xBD };

    data_t int64Zero{ 0x00 };
    data_t int64Positive{ 0x1B, 0x00, 0x2B, 0xDC, 0x54, 0x5D, 0xF2, 0xBD, 0xED };
    data_t int64Negative{ 0x3B, 0x0, 0x02B, 0xDC, 0x54, 0x5D, 0xF2, 0xBD, 0xEC };

    data_t uint64Zero{ 0x00 };
    data_t uint64Positive1{ 0x1B, 0x00, 0x2B, 0xDC, 0x54, 0x5D, 0xF2, 0xBD, 0xED };
    data_t uint64Positive2{ 0x1B, 0xFF, 0xD4, 0x23, 0xAB, 0xA2, 0x0D, 0x42, 0x13  };

    data_t float32Zero{ 0xFA, 0x00, 0x00, 0x00, 0x00 };
    data_t float32Positive{ 0xFA, 0x40, 0x49, 0x0E, 0x56 };
    data_t float32Negative{ 0xFA, 0xC0, 0x2D, 0xF8, 0xA1 };

    data_t float64Zero{ 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    data_t float64Positive{ 0xFB, 0x40, 0x09, 0x21, 0xFB, 0x54, 0x44, 0x2E, 0xEA };
    data_t float64Negative{ 0xFB, 0xC0, 0x05, 0xBF, 0x0A, 0x8B, 0x14, 0x5F, 0xCF };

    data_t propertySetNone{ 0x00 };
    data_t propertySetAll{ 0x1A, 0xFF, 0xFF, 0xFF, 0xFF };
    data_t propertySetMixed1{ 0x1A, 0xAA, 0xFF, 0x00, 0x55 };

    data_t timePoint1{ 0xFB, 0x41, 0xD7, 0x9A, 0x54, 0xCB, 0x60, 0x00, 0x00 };
    data_t steadyTimePoint1{ 0xFB, 0x41, 0x13, 0xD1, 0x78, 0x80, 0x00, 0x00, 0x00 };

    data_t duration1{ 0xFB, 0x40, 0x5E, 0xDD, 0x2F, 0x1A, 0x9F, 0xBE, 0x77 };
    data_t duration2{ 0xFB, 0x40, 0x75, 0x61, 0x2B, 0x02, 0x0C, 0x49, 0xBA };

    data_t uuid1{ 0x50, 0x8C, 0x96, 0x14, 0x8E, 0x58, 0xBD, 0x11, 0xEB, 0xAE, 0x93, 0x02, 0x42, 0xAC, 0x13, 0x00, 0x02 };

    data_t string1{ 0x66, 0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72 };
    data_t string2{ 0x6D, 0x22, 0x66, 0x6F, 0x6F, 0x22, 0x20, 0x62, 0x61, 0x72, 0x20, 0x62, 0x61, 0x7A };
    data_t string3{ 0x6D, 0x66, 0x6F, 0x6F, 0x20, 0x22, 0x62, 0x61, 0x72, 0x22, 0x20, 0x62, 0x61, 0x7A };
    data_t string4{ 0x6D, 0x66, 0x6F, 0x6F, 0x20, 0x62, 0x61, 0x72, 0x20, 0x22, 0x62, 0x61, 0x7A, 0x22 };
    data_t string5{ 0x70, 0x66, 0x6F, 0x6F, 0x5C, 0x20, 0x62, 0x61, 0x72, 0xC2, 0xA9, 0x0A, 0x20, 0x62, 0x5C, 0x61, 0x7A };

    //
    // enum
    //

    data_t enum1{ 0x05 };

    //
    // property
    //

    data_t structSimple1_int32Property = Concat(0x01, int32Positive);
    data_t structSimple1_stringProperty = Concat(0x02, string1);
    data_t structSimple1_float32Property = Concat(0x04, float32Positive);

    data_t structComplex1_enumProperty = Concat(0x07, enum1);
    data_t structComplex1_float64Property = Concat(0x04, float64Negative);
    data_t structComplex1_timepointProperty = Concat(0x18, 0x19, timePoint1);
    data_t structComplex1_structSimpleProperty = Concat(0x0F, 0xA1, 0x03, boolFalse);

    data_t structComplex2_propertySetProperty = Concat(0x03, propertySetMixed1);
    data_t structComplex2_durationVectorProperty = Concat(0x09, 0x82, duration1, duration2);
    data_t structComplex2_uuidProperty = Concat(0x06, uuid1);

    //
    // vector
    //

    data_t vectorBool = Concat(0x83, boolTrue, boolFalse, boolFalse);
    data_t vectorFloat = Concat(0x82, float32Positive, float32Negative);
    data_t vectorStructSimple = Concat(0x82, 0xA1, 0x01, int32Positive, 0xA1, 0x03, boolFalse);
    data_t vectorEmpty = Concat(0x80);

    //
    // struct
    //

    data_t structSimple1_Valid = Concat(
        0xA3,
        structSimple1_int32Property,
        structSimple1_stringProperty,
        structSimple1_float32Property
    );

    data_t structSimple1_All = Concat(
        structSimple1_Valid
    );

    data_t structSimple1_Specific = Concat(
        0xA1,
        structSimple1_float32Property
    );

    data_t structSimple1_None = Concat(
        0xA0
    );
    
    data_t structComplex1_Valid = Concat(
        0xA4,
        structComplex1_enumProperty,
        structComplex1_float64Property,
        structComplex1_timepointProperty,
        structComplex1_structSimpleProperty
    );

    data_t structComplex1_Specific = Concat(
        0xA1,
        structComplex1_timepointProperty
    );

    data_t structComplex2_Valid = Concat(
        0xA3,
        structComplex2_propertySetProperty,
        structComplex2_durationVectorProperty,
        structComplex2_uuidProperty
    );

    data_t structComplex2_Specific = Concat(
        0xA1,
        structComplex2_durationVectorProperty
    );

    //
    // consecutive
    //

    data_t consecutiveTypes1 = Concat(
        structSimple1_Valid,
        structComplex1_Valid,
        vectorBool,
        structComplex2_Valid
    );

    //
    // tuple
    //

    data_t serializationTuple1 = Concat(
        0x9F,
        string1,
        enum1,
        vectorBool,
        structSimple1_Valid,
        0xFF
    );

    //
    // unknown properties
    //

    data_t structSimple1_unknownProperty = Concat(0x05, string5);
    data_t structComplex1_unknownProperty = Concat(0x05, structComplex2_Valid);

    data_t structSimple1_Unknown = Concat(
        0xA4,
        structSimple1_int32Property,
        structSimple1_stringProperty,
        structSimple1_float32Property,
        structSimple1_unknownProperty
    );

    data_t structComplex1_Unknown = Concat(
        0xA5,
        structComplex1_enumProperty,
        structComplex1_float64Property,
        structComplex1_unknownProperty,
        structComplex1_timepointProperty,
        structComplex1_structSimpleProperty
    );
};

INSTANTIATE_TYPED_TEST_SUITE_P(TestCborSerializer, TestSerializer, CborSerializerTestDataEncoded);

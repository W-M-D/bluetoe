#include <iostream>

#include <bluetoe/characteristic.hpp>

#define BOOST_TEST_MODULE
#include <boost/test/included/unit_test.hpp>
#include "hexdump.hpp"

namespace {
    std::uint32_t       simple_value       = 0xaabbccdd;
    const std::uint32_t simple_const_value = 0xaabbccdd;

    typedef bluetoe::characteristic<
        bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
        bluetoe::bind_characteristic_value< std::uint32_t, &simple_value >
    > simple_char;

   typedef bluetoe::characteristic<
        bluetoe::characteristic_uuid16< 0xD0B1 >,
        bluetoe::bind_characteristic_value< std::uint32_t, &simple_value >
    > short_uuid_char;

    typedef bluetoe::characteristic<
        bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
        bluetoe::bind_characteristic_value< const std::uint32_t, &simple_const_value >
    > simple_const_char;

    template < typename Char >
    class access_attributes : public Char
    {
    public:
        bluetoe::details::attribute attribute_by_type( std::uint16_t type )
        {
            for ( int index = 0; index != this->number_of_attributes; ++index )
            {
                const bluetoe::details::attribute value_attribute = this->attribute_at( index );

                if ( value_attribute.uuid == type )
                {
                    return value_attribute;
                }
            }

            BOOST_REQUIRE( !"Type not found" );

            return bluetoe::details::attribute{ 0, 0 };
        }

        void compare_characteristic_at( const std::initializer_list< std::uint8_t >& input, std::size_t index )
        {
            compare_characteristic_impl( input, this->attribute_at( index ) );
        }

        void compare_characteristic( const std::initializer_list< std::uint8_t >& input, std::uint16_t type )
        {
            compare_characteristic_impl( input, attribute_by_type( type ) );
        }

    private:
        void compare_characteristic_impl( const std::initializer_list< std::uint8_t >& input, const bluetoe::details::attribute& value_attribute )
        {
            const std::vector< std::uint8_t > values( input );

            std::uint8_t buffer[ 1000 ];
            auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

            BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );
            BOOST_REQUIRE_EQUAL_COLLECTIONS( values.begin(), values.end(), &read.buffer[ 0 ], &read.buffer[ read.buffer_size ] );
        }
    };

    template < typename Char >
    struct read_characteristic_properties : Char
    {
        read_characteristic_properties()
        {
            const bluetoe::details::attribute value_attribute = this->attribute_at( 0 );
            std::uint8_t buffer[ 100 ];
            auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

            BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );
            properties = buffer[ 0 ];
        }

        std::uint8_t properties;
    };

}

BOOST_AUTO_TEST_CASE( even_the_simplest_characteristic_has_at_least_2_attributes )
{
    BOOST_CHECK_GE( std::size_t( simple_char::number_of_attributes ), 2 );
}

BOOST_FIXTURE_TEST_CASE( the_first_attribute_is_the_characteristic_declaration, simple_char )
{
    const bluetoe::details::attribute char_declaration = attribute_at( 0 );

    BOOST_CHECK_EQUAL( char_declaration.uuid, 0x2803 );
}

BOOST_AUTO_TEST_SUITE( characteristic_declaration_access )

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_has_a_length_of_19, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_REQUIRE( char_declaration.access );
        char_declaration.access( read, 1 );

        BOOST_CHECK_EQUAL( read.buffer_size, 19u );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_has_a_length_of_17_with_offset_2, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 2 );

        BOOST_REQUIRE( char_declaration.access );
        char_declaration.access( read, 1 );

        BOOST_CHECK_EQUAL( read.buffer_size, 17u );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_has_a_length_of_0_with_offset_19, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 19 );

        BOOST_REQUIRE( char_declaration.access );
        BOOST_CHECK( char_declaration.access( read, 1 ) == bluetoe::details::attribute_access_result::success );

        BOOST_CHECK_EQUAL( read.buffer_size, 0 );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_returns_invalid_offset_when_read_behind_data, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 20 );

        BOOST_CHECK( char_declaration.access( read, 1 ) == bluetoe::details::attribute_access_result::invalid_offset );
    }

    BOOST_FIXTURE_TEST_CASE( the_short_characteristic_declaration_has_a_length_of_5, short_uuid_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        char_declaration.access( read, 1 );

        BOOST_CHECK_EQUAL( read.buffer_size, 5u );
    }

    BOOST_FIXTURE_TEST_CASE( the_short_characteristic_declaration_has_a_length_of_4_with_offset_1, short_uuid_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 1 );

        char_declaration.access( read, 1 );

        BOOST_CHECK_EQUAL( read.buffer_size, 4u );
    }

    BOOST_FIXTURE_TEST_CASE( the_short_characteristic_declaration_has_a_length_of_1_with_offset_4, short_uuid_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 4 );

        char_declaration.access( read, 1 );

        BOOST_CHECK_EQUAL( read.buffer_size, 1u );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_contains_the_uuid, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::success == char_declaration.access( read, 1 ) );

        // D0B10674-6DDD-4B59-89CA-A009B78C956B
        static const std::uint8_t expected_uuid[] = { 0x6B, 0x95, 0x8C, 0xB7, 0x09, 0xA0, 0xCA, 0x89, 0x59, 0x4B, 0xDD, 0x6D, 0x74, 0x06, 0xB1, 0xD0 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_uuid ), std::end( expected_uuid ), &buffer[ 3 ], &buffer[ 19 ] );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_contains_the_uuid_at_offset_3, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 3 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::success == char_declaration.access( read, 1 ) );

        // D0B10674-6DDD-4B59-89CA-A009B78C956B
        static const std::uint8_t expected_uuid[] = { 0x6B, 0x95, 0x8C, 0xB7, 0x09, 0xA0, 0xCA, 0x89, 0x59, 0x4B, 0xDD, 0x6D, 0x74, 0x06, 0xB1, 0xD0 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_uuid ), std::end( expected_uuid ), &buffer[ 0 ], &buffer[ 16 ] );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_contains_the_value_handle, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::success == char_declaration.access( read, 0x1234 ) );

        static const std::uint8_t expected_handle[] = { 0x35, 0x12 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_handle ), std::end( expected_handle ), &buffer[ 1 ], &buffer[ 3 ] );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_contains_the_value_handle_at_offset_1, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 1 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::success == char_declaration.access( read, 0x1234 ) );

        static const std::uint8_t expected_handle[] = { 0x35, 0x12 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_handle ), std::end( expected_handle ), &buffer[ 0 ], &buffer[ 2 ] );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_contains_the_hibyte_of_the_value_handle_at_offset_2, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 2 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::success == char_declaration.access( read, 0x1234 ) );

        BOOST_CHECK_EQUAL( buffer[ 0 ], 0x12 );
    }

    BOOST_FIXTURE_TEST_CASE( read_char_declaration_buffer_to_small, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 17 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::read_truncated == char_declaration.access( read, 1 ) );
        BOOST_CHECK_EQUAL( read.buffer_size, 17u );
        static const std::uint8_t expected_uuid[] = { 0x6B, 0x95, 0x8C, 0xB7, 0x09, 0xA0, 0xCA, 0x89, 0x59, 0x4B, 0xDD, 0x6D, 0x74, 0x06 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_uuid ), std::end( expected_uuid ), &buffer[ 3 ], &buffer[ 17 ] );
    }

    BOOST_FIXTURE_TEST_CASE( read_char_declaration_buffer_to_small_with_offset_1, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 17 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 1 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::read_truncated == char_declaration.access( read, 1 ) );
        BOOST_CHECK_EQUAL( read.buffer_size, 17u );
        static const std::uint8_t expected_uuid[] = { 0x6B, 0x95, 0x8C, 0xB7, 0x09, 0xA0, 0xCA, 0x89, 0x59, 0x4B, 0xDD, 0x6D, 0x74, 0x06, 0xB1 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_uuid ), std::end( expected_uuid ), &buffer[ 2 ], &buffer[ 17 ] );
    }

    BOOST_FIXTURE_TEST_CASE( the_characteristic_declaration_contains_the_16bit_uuid, short_uuid_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::success == char_declaration.access( read, 1 ) );

        static const std::uint8_t expected_uuid[] = { 0xB1, 0xD0 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_uuid ), std::end( expected_uuid ), &buffer[ 3 ], &buffer[ 5 ] );
    }

    BOOST_FIXTURE_TEST_CASE( char_declaration_is_not_writable, simple_char )
    {
        const bluetoe::details::attribute char_declaration = attribute_at( 0 );
        std::uint8_t buffer[ 4 ];
        auto write = bluetoe::details::attribute_access_arguments::write( buffer );

        BOOST_CHECK( char_declaration.access( write, 1 ) == bluetoe::details::attribute_access_result::write_not_permitted );
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( characteristic_value_access )

    BOOST_FIXTURE_TEST_CASE( simple_value_can_be_read, simple_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );

        static const std::uint8_t expected_value[] = { 0xdd, 0xcc, 0xbb, 0xaa };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_value ), std::end( expected_value ), read.buffer, read.buffer + read.buffer_size );
    }

    BOOST_FIXTURE_TEST_CASE( char_value_returns_invalid_offset_when_read_behind_the_data, simple_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 5 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::invalid_offset == value_attribute.access( read, 1 ) );
    }

    BOOST_FIXTURE_TEST_CASE( simple_value_can_be_read_with_offset, simple_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 2 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );

        static const std::uint8_t expected_value[] = { 0xbb, 0xaa };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_value ), std::end( expected_value ), read.buffer, read.buffer + read.buffer_size );
    }

    BOOST_FIXTURE_TEST_CASE( simple_value_can_be_read_with_offset_equal_to_length, simple_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 4 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );
        BOOST_CHECK_EQUAL( read.buffer_size, 0 );
    }

    BOOST_FIXTURE_TEST_CASE( simple_value_can_be_written, simple_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t new_value[] = { 0x01, 0x02, 0x03, 0x04 };

        auto write = bluetoe::details::attribute_access_arguments::write( new_value );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( write, 1 ) );

        std::uint8_t buffer[ 4 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );

        static const std::uint8_t expected_value[] = { 0x01, 0x02, 0x03, 0x04 };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_value ), std::end( expected_value ), read.buffer, read.buffer + read.buffer_size );
    }

    BOOST_FIXTURE_TEST_CASE( simple_const_value_can_be_read, simple_const_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == value_attribute.access( read, 1 ) );

        static const std::uint8_t expected_value[] = { 0xdd, 0xcc, 0xbb, 0xaa };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_value ), std::end( expected_value ), read.buffer, read.buffer + read.buffer_size );
    }

    BOOST_FIXTURE_TEST_CASE( simple_const_value_can_not_be_writte, simple_const_char )
    {
        const bluetoe::details::attribute value_attribute = attribute_at( 1 );
        std::uint8_t new_value[] = { 0x01, 0x02, 0x03, 0x04 };

        auto write = bluetoe::details::attribute_access_arguments::write( new_value );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::write_not_permitted == value_attribute.access( write, 1 ) );
    }

    BOOST_AUTO_TEST_CASE( simple_value_without_read_access_can_not_be_read )
    {
        bluetoe::characteristic<
            bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
            bluetoe::bind_characteristic_value< std::uint32_t, &simple_value >,
            bluetoe::no_read_access
        > simple_char_without_write_access;

        const bluetoe::details::attribute value_attribute = simple_char_without_write_access.attribute_at( 1 );

        std::uint8_t old_value[ 4 ];

        auto read = bluetoe::details::attribute_access_arguments::read( old_value, 0 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::read_not_permitted == value_attribute.access( read, 1 ) );
    }

    BOOST_AUTO_TEST_CASE( simple_value_without_write_access_can_not_be_written )
    {
        bluetoe::characteristic<
            bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
            bluetoe::bind_characteristic_value< std::uint32_t, &simple_value >,
            bluetoe::no_write_access
        > simple_char_without_read_access;

        const bluetoe::details::attribute value_attribute = simple_char_without_read_access.attribute_at( 1 );

        std::uint8_t new_value[] = { 0x01, 0x02, 0x03, 0x04 };

        auto write = bluetoe::details::attribute_access_arguments::write( new_value );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::write_not_permitted == value_attribute.access( write, 1 ) );
    }

    static std::uint32_t write_to_large_value = 15;

    BOOST_AUTO_TEST_CASE( write_to_large )
    {
        bluetoe::characteristic<
            bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
            bluetoe::bind_characteristic_value< std::uint32_t, &write_to_large_value >
        > characteristic;

        std::uint8_t new_value[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };

        auto write = bluetoe::details::attribute_access_arguments::write( new_value );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::write_overflow == characteristic.attribute_at( 1 ).access( write, 1 ) );
        BOOST_CHECK_EQUAL( write_to_large_value, 15 );
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( characteristic_properties )

    BOOST_FIXTURE_TEST_CASE( by_default_an_attribute_is_readable_and_writeable, read_characteristic_properties< simple_char > )
    {
        BOOST_CHECK( properties & 0x02 );
        BOOST_CHECK( properties & 0x08 );
    }

    BOOST_FIXTURE_TEST_CASE( read_only, read_characteristic_properties< simple_const_char > )
    {
        BOOST_CHECK( properties & 0x02 );
        BOOST_CHECK_EQUAL( properties & 0x08, 0 );
    }

    typedef bluetoe::characteristic<
            bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
            bluetoe::bind_characteristic_value< std::uint32_t, &simple_value >,
            bluetoe::no_write_access
        > simple_char_without_write_access;

    BOOST_FIXTURE_TEST_CASE( write_only, read_characteristic_properties< simple_char_without_write_access > )
    {
        BOOST_CHECK( properties & 0x02 );
        BOOST_CHECK_EQUAL( properties & 0x08, 0 );
    }

    typedef bluetoe::characteristic<
            bluetoe::characteristic_uuid< 0xD0B10674, 0x6DDD, 0x4B59, 0x89CA, 0xA009B78C956B >,
            bluetoe::bind_characteristic_value< std::uint32_t, &simple_value >,
            bluetoe::no_write_access,
            bluetoe::no_read_access
        > simple_char_without_any_access;

    BOOST_FIXTURE_TEST_CASE( only, read_characteristic_properties< simple_char_without_any_access > )
    {
        BOOST_CHECK_EQUAL( properties & 0x02, 0 );
        BOOST_CHECK_EQUAL( properties & 0x08, 0 );
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( characteristic_extended_properties )

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( characteristic_user_description )

    char simple_value = 0;
    const char name[] = "Die ist der Name";

    typedef bluetoe::characteristic<
        bluetoe::characteristic_name< name >,
        bluetoe::characteristic_uuid16< 0x0815 >,
        bluetoe::bind_characteristic_value< char, &simple_value >
    > named_char;

    BOOST_FIXTURE_TEST_CASE( there_is_an_attribute_with_the_given_name, access_attributes< named_char > )
    {
        BOOST_CHECK_EQUAL( 3, int(number_of_attributes) );
        compare_characteristic( { 'D', 'i', 'e', ' ', 'i', 's', 't', ' ', 'd', 'e', 'r', ' ', 'N', 'a', 'm', 'e' }, 0x2901 );
    }

    BOOST_FIXTURE_TEST_CASE( characteristic_user_description_is_readable_with_offset, access_attributes< named_char > )
    {
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 12 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == attribute_by_type( 0x2901 ).access( read, 1 ) );

        static const std::uint8_t expected_value[] = { 'N', 'a', 'm', 'e' };
        BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( expected_value ), std::end( expected_value ), read.buffer, read.buffer + read.buffer_size );
    }

    BOOST_FIXTURE_TEST_CASE( characteristic_user_description_is_readable_with_offset_16, access_attributes< named_char > )
    {
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 16 );

        BOOST_REQUIRE( bluetoe::details::attribute_access_result::success == attribute_by_type( 0x2901 ).access( read, 1 ) );
        BOOST_CHECK_EQUAL( read.buffer_size, 0 );
    }

    BOOST_FIXTURE_TEST_CASE( characteristic_user_description_is_not_readable_behind, access_attributes< named_char > )
    {
        std::uint8_t buffer[ 100 ];
        auto read = bluetoe::details::attribute_access_arguments::read( buffer, 17 );

        BOOST_CHECK( bluetoe::details::attribute_access_result::invalid_offset == attribute_by_type( 0x2901 ).access( read, 1 ) );
    }

    BOOST_FIXTURE_TEST_CASE( is_not_writeable, access_attributes< named_char > )
    {
        static std::uint8_t buffer[ 100 ];

        auto attr  = attribute_by_type( 0x2901 );
        auto write = bluetoe::details::attribute_access_arguments::write( buffer );

        BOOST_CHECK( attr.access( write, 1 ) == bluetoe::details::attribute_access_result::write_not_permitted );
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( client_characteristic_configuration )

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( server_characteristic_configuration )

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( characteristic_presentation_format )

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( characteristic_aggregate_format )

BOOST_AUTO_TEST_SUITE_END()

#include <iostream>
#include "test_services.hpp"

#define BOOST_TEST_MODULE
#include <boost/test/included/unit_test.hpp>

#include <iterator>

BOOST_AUTO_TEST_CASE( service_without_any_characteristic_results_in_one_attribute )
{
    const auto number = empty_service::number_of_attributes;
    BOOST_CHECK_EQUAL( 1u, number );
}

BOOST_AUTO_TEST_CASE( first_attribute_is_the_primary_service )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );
    BOOST_CHECK_EQUAL( 0x2800, attr.uuid );
}

static const std::uint8_t global_temperature_service_uuid[ 16 ] = { 0x2A, 0xD9, 0x91, 0x11, 0xAB, 0x5B, 0x58, 0xB0, 0x3B, 0x4F, 0x50, 0x44, 0x52, 0x6E, 0x42, 0xF0 };

static void check_service_uuid( const bluetoe::details::attribute_access_arguments& args, std::size_t size = sizeof( global_temperature_service_uuid ) )
{
    BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( global_temperature_service_uuid ), std::begin( global_temperature_service_uuid ) + size, args.buffer, args.buffer + args.buffer_size );
}

BOOST_AUTO_TEST_CASE( first_attribute_is_the_primary_service_and_can_be_read )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );
    BOOST_REQUIRE( attr.access );

    std::uint8_t buffer[ 16 ];
    auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );
    const auto access_result = attr.access( read, 1 );

    BOOST_CHECK( access_result == bluetoe::details::attribute_access_result::success );
    check_service_uuid( read );
}

BOOST_AUTO_TEST_CASE( first_attribute_is_the_primary_service_and_can_be_read_buffer_larger )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );

    std::uint8_t buffer[ 20 ];
    auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );
    const auto access_result = attr.access( read, 1 );

    BOOST_CHECK( access_result == bluetoe::details::attribute_access_result::success );
    check_service_uuid( read );
}

BOOST_AUTO_TEST_CASE( first_attribute_is_the_primary_service_and_can_be_read_buffer_larger_with_offset )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );

    std::uint8_t buffer[ 20 ];
    auto read = bluetoe::details::attribute_access_arguments::read( buffer, 4 );
    const auto access_result = attr.access( read, 1 );

    BOOST_CHECK( access_result == bluetoe::details::attribute_access_result::success );
    BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( global_temperature_service_uuid ) + 4, std::end( global_temperature_service_uuid ),
        &read.buffer[ 0 ], &read.buffer[ read.buffer_size ]);
}

BOOST_AUTO_TEST_CASE( first_attribute_is_the_primary_service_and_can_be_read_buffer_larger_with_offset_16 )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );

    std::uint8_t buffer[ 20 ];
    auto read = bluetoe::details::attribute_access_arguments::read( buffer, 16 );
    const auto access_result = attr.access( read, 1 );

    BOOST_CHECK( access_result == bluetoe::details::attribute_access_result::success );
    BOOST_CHECK_EQUAL( 0, read.buffer_size );
}

BOOST_AUTO_TEST_CASE( first_attribute_is_the_primary_service_and_can_be_read_buffer_to_small )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );

    std::uint8_t buffer[ 15 ];
    auto read = bluetoe::details::attribute_access_arguments::read( buffer, 0 );
    const auto access_result = attr.access( read, 1 );

    BOOST_CHECK( access_result == bluetoe::details::attribute_access_result::read_truncated );
    check_service_uuid( read, sizeof( buffer ) );
}

BOOST_AUTO_TEST_CASE( write_to_primary_service )
{
    const auto attr = empty_service::attribute_at< 0 >( 0 );

    std::uint8_t buffer[] = { 1, 2, 3 };
    auto write = bluetoe::details::attribute_access_arguments::write( buffer );
    const auto access_result = attr.access( write, 1 );

    BOOST_CHECK( access_result == bluetoe::details::attribute_access_result::write_not_permitted );
}

BOOST_FIXTURE_TEST_CASE( accessing_all_attributes, service_with_3_characteristics )
{
    static constexpr std::size_t expected_number_of_attributes = 7u;
    BOOST_REQUIRE_EQUAL( unsigned( number_of_attributes ), expected_number_of_attributes );

    BOOST_CHECK_EQUAL( 0x2800, attribute_at< 0 >( 0 ).uuid );
    BOOST_CHECK_EQUAL( 0x2803, attribute_at< 0 >( 1 ).uuid );
    BOOST_CHECK_EQUAL( 0x0001, attribute_at< 0 >( 2 ).uuid );
    BOOST_CHECK_EQUAL( 0x2803, attribute_at< 0 >( 3 ).uuid );
    BOOST_CHECK_EQUAL( 0x0001, attribute_at< 0 >( 4 ).uuid );
    BOOST_CHECK_EQUAL( 0x2803, attribute_at< 0 >( 5 ).uuid );
    BOOST_CHECK_EQUAL( 0x0815, attribute_at< 0 >( 6 ).uuid );
}

BOOST_FIXTURE_TEST_CASE( read_by_group_type_response, service_with_3_characteristics )
{
    std::uint8_t    buffer[ 100 ];

    std::uint8_t* const end = read_primary_service_response( std::begin( buffer ), std::end( buffer ), 12, true );

    BOOST_CHECK_EQUAL( end - std::begin( buffer ), 20u );

    static const std::uint8_t expected_result[] =
    {
        0x0c, 0x00,              // Starting Handle
        0x12, 0x00,              // Ending Handle
        0xA9, 0x3C, 0xC7, 0x5B,  // Attribute Value == 128 bit UUID
        0xED, 0x4E, 0x8A, 0xA2,
        0x9F, 0x49, 0xE2, 0x0D,
        0x94, 0x40, 0x8B, 0x8C
    };

    BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( buffer ), end, std::begin( expected_result ), std::end( expected_result ) );
}

BOOST_FIXTURE_TEST_CASE( read_by_group_type_response_buffer_to_small, service_with_3_characteristics )
{
    std::uint8_t    buffer[ 19 ];
    std::uint16_t   index = 1;

    std::uint8_t* const end = read_primary_service_response( std::begin( buffer ), std::end( buffer ), index, true );

    BOOST_CHECK( end == std::begin( buffer ) );
}

BOOST_FIXTURE_TEST_CASE( read_by_group_type_response_for_16bit_uuid, cycling_speed_and_cadence_service )
{
    std::uint8_t    buffer[ 100 ];

    std::uint8_t* const end = read_primary_service_response( std::begin( buffer ), std::end( buffer ), 1, false );

    BOOST_CHECK_EQUAL( end - std::begin( buffer ), 6u );

    static const std::uint8_t expected_result[] =
    {
        0x01, 0x00,   // Starting Handle
        0x05, 0x00,   // Ending Handle
        0x16, 0x18    // Attribute Value == 16 bit UUID
    };

    BOOST_CHECK_EQUAL_COLLECTIONS( std::begin( buffer ), end, std::begin( expected_result ), std::end( expected_result ) );
}

BOOST_FIXTURE_TEST_CASE( primary_service_value_compare_128bit, global_temperature_service )
{
    auto compare = bluetoe::details::attribute_access_arguments::compare_value( &global_temperature_service_uuid[ 0 ], &global_temperature_service_uuid[ sizeof global_temperature_service_uuid ]);
    BOOST_CHECK( attribute_at< 0 >( 0 ).access( compare, 1 ) == bluetoe::details::attribute_access_result::value_equal );
}

BOOST_AUTO_TEST_SUITE( number_of_client_configs )

BOOST_AUTO_TEST_CASE( without_notifications_there_is_no_demand_for_client_configurations )
{
    BOOST_CHECK_EQUAL( 0, int( service_with_3_characteristics::number_of_client_configs ) );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( find_notification_data )

char v1, v2, v3;

typedef bluetoe::service<
    bluetoe::service_uuid16< 0x8C8B >,
    bluetoe::characteristic<
        bluetoe::characteristic_uuid16< 0x8C8B >,
        bluetoe::bind_characteristic_value< decltype( v1 ), &v1 >,
        bluetoe::notify
    >,
    bluetoe::characteristic<
        bluetoe::characteristic_uuid16< 0x8C8C >,
        bluetoe::bind_characteristic_value< decltype( v2 ), &v2 >
    >,
    bluetoe::characteristic<
        bluetoe::characteristic_uuid16< 0x8C8D >,
        bluetoe::bind_characteristic_value< decltype( v3 ), &v3 >,
        bluetoe::notify
    >
> service_with_2_notifications;

BOOST_AUTO_TEST_CASE( service_with_2_notifications_has_2_client_configurations )
{
    BOOST_CHECK_EQUAL( 2, int( service_with_2_notifications::number_of_client_configs ) );
}

// just to be sure
BOOST_FIXTURE_TEST_CASE( check_service_with_2_notifications_attribute_layout, service_with_2_notifications )
{
    BOOST_CHECK_EQUAL( 0x2800, attribute_at< 0 >( 0 ).uuid );

    BOOST_CHECK_EQUAL( 0x2803, attribute_at< 0 >( 1 ).uuid );
    BOOST_CHECK_EQUAL( 0x8C8B, attribute_at< 0 >( 2 ).uuid );
    BOOST_CHECK_EQUAL( 0x2902, attribute_at< 0 >( 3 ).uuid );

    BOOST_CHECK_EQUAL( 0x2803, attribute_at< 0 >( 4 ).uuid );
    BOOST_CHECK_EQUAL( 0x8C8C, attribute_at< 0 >( 5 ).uuid );

    BOOST_CHECK_EQUAL( 0x2803, attribute_at< 0 >( 6 ).uuid );
    BOOST_CHECK_EQUAL( 0x8C8D, attribute_at< 0 >( 7 ).uuid );
    BOOST_CHECK_EQUAL( 0x2902, attribute_at< 0 >( 8 ).uuid );
}

BOOST_FIXTURE_TEST_CASE( notification_data_not_found, service_with_2_notifications )
{
    BOOST_CHECK( !( find_notification_data< 1, 0 >( nullptr ).valid() ) );
}

BOOST_FIXTURE_TEST_CASE( notification_data_found_first_char, service_with_2_notifications )
{
    const auto result1 = find_notification_data< 1, 0 >( &v1 );

    BOOST_REQUIRE( result1.valid() );
    BOOST_CHECK_EQUAL( result1.handle(), 3 );
    BOOST_CHECK_EQUAL( result1.value_attribute().uuid, 0x8C8B );
    BOOST_CHECK_EQUAL( result1.client_characteristic_configuration_index(), 0 );
}

BOOST_FIXTURE_TEST_CASE( notification_data_found_second_char, service_with_2_notifications )
{
    const auto result2 = find_notification_data< 1, 0 >( &v3 );

    BOOST_REQUIRE( result2.valid() );
    BOOST_CHECK_EQUAL( result2.handle(), 8 );
    BOOST_CHECK_EQUAL( result2.value_attribute().uuid, 0x8C8D );
    BOOST_CHECK_EQUAL( result2.client_characteristic_configuration_index(), 1 );
}

BOOST_FIXTURE_TEST_CASE( characteristic_without_notification_cant_be_found, service_with_2_notifications )
{
    BOOST_CHECK( !( find_notification_data< 1, 0 >( &v2 ).valid() ) );
}

BOOST_AUTO_TEST_SUITE_END()
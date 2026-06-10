cmake_minimum_required( VERSION 3.16.0 )

set( requires "Core2-for-AWS-IoT-Kit" )
set( include_dirs "include" )

# Pull in the PaHUB component only when I2C multiplexing is enabled.
if( CONFIG_SEED_LASER_PM25_USE_PAHUB )
    list( APPEND requires "core2foraws-unit-pahub" )
    list( APPEND include_dirs "../core2foraws-unit-pahub/include" )
endif()

idf_component_register(
    SRCS "seed_laser_pm25.c"
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${requires}
)

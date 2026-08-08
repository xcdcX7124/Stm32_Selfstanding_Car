set(src_dirs
            BLETOJDY
            JOYSTICK
            OLED
            KEY)

set(include_dirs
            BLETOJDY
            JOYSTICK
            OLED
            KEY)

set(requires
            bt
            nvs_flash
            esp_adc
            driver)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)